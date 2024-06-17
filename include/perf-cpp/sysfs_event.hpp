#include <perf-cpp/perf_event.hpp>
#include <perf-cpp/util.hpp>

namespace perf_cpp
{

static constexpr std::uint64_t apply_mask(std::uint64_t value, std::uint64_t mask)
{
    std::uint64_t res = 0;
    for (int mask_bit = 0, value_bit = 0; mask_bit < 64; mask_bit++)
    {
        if (mask & 1ULL << mask_bit)
        {
            res |= ((value >> value_bit) & 1) << mask_bit;
            value_bit++;
        }
    }
    return res;
}

static std::uint64_t parse_bitmask(const std::string& format)
{
    enum BITMASK_REGEX_GROUPS
    {
        BM_WHOLE_MATCH,
        BM_BEGIN,
        BM_END,
    };

    std::uint64_t mask = 0x0;

    static const std::regex bit_mask_regex(R"((\d+)?(?:-(\d+)))");
    const std::sregex_iterator end;
    std::smatch bit_mask_match;
    for (std::sregex_iterator i = { format.begin(), format.end(), bit_mask_regex }; i != end; ++i)
    {
        const auto& match = *i;
        int start = std::stoi(match[BM_BEGIN]);
        int end = (match[BM_END].length() == 0) ? start : std::stoi(match[BM_END]);

        const auto len = (end + 1) - start;
        if (start < 0 || end > 63 || len > 64)
        {
            return 0;
        }

        /* Set `len` bits and shift them to where they should start.
         * 4-bit example: format "1-3" produces mask 0b1110.
         *    start := 1, end := 3
         *    len  := 3 + 1 - 1 = 3
         *    bits := bit(3) - 1 = 0b1000 - 1 = 0b0111
         *    mask := 0b0111 << 1 = 0b1110
         * */

        // Shifting by 64 bits causes undefined behaviour, so in this case set
        // all bits by assigning the maximum possible value for std::uint64_t.
        const std::uint64_t bits =
            (len == 64) ? std::numeric_limits<std::uint64_t>::max() : (1ULL << len) - 1;

        mask |= bits << start;
    }
    Log::debug() << std::showbase << std::hex << "config mask: " << format << " = " << mask
                 << std::dec << std::noshowbase;
    return mask;
}

template <typename T>
T read_file_or_else(std::string filename, T or_else)
{
    T val;
    std::ifstream stream(filename);
    stream >> val;
    if (stream.fail())
    {
        return or_else;
    }
    return val;
}

class SysfsPerfEvent : public PerfEvent
{
public:
    SysfsPerfEvent(std::string pmu_name, std::string event_name) : PerfEvent()
    {
        // TODO: This feels 10x as complicated as it ought to be. Rework it.
        // Parse event description //

        /* Event description format:
         *   Name of a Performance Monitoring Unit (PMU) and an event name,
         *   separated by either '/' or ':' (for perf-like syntax);  followed by an
         *   optional separator:
         *
         *     <pmu>/<event_name>[/]
         *   OR
         *     <pmu>:<event_name>[/]
         *
         *   Examples (both specify the same event):
         *
         *     cpu/cache-misses/
         *     cpu:cache-misses
         *
         * */

        Log::debug() << "parsing event description: pmu='" << pmu_name << "', event='" << event_name
                     << "'";

        const std::filesystem::path pmu_path =
            std::filesystem::path("/sys/bus/event_source/devices") / pmu_name;

        // read PMU type id
        if ((attr_.type = read_file_or_else<uint64_t>(pmu_path / "type", -1)) == -1)
        {
            return;
        }

        // If the processor is heterogenous, "cpus" contains the cores that support this PMU. If the
        // PMU is an uncore PMU "cpumask" contains the cores that are logically assigned to that
        // PMU. Why there need to be two seperate files instead of one, nobody knows, but simply
        // parse both.
        std::set<Cpu> cpus;
        auto cpuids = parse_list_from_file(pmu_path / "cpus");

        if (cpuids.empty())
        {
            cpuids = parse_list_from_file(pmu_path / "cpumask");
        }

        std::transform(cpuids.begin(), cpuids.end(), std::inserter(cpus, cpus.end()),
                       [](uint32_t cpuid) { return Cpu(cpuid); });

        if (cpus.empty())
        {
            availability_ = AnyLocation();
        }
        else
        {
            availability_ = cpus;
        }

        std::filesystem::path event_path = pmu_path / "events" / event_name;

        // read event configuration
        std::string ev_cfg;
        if ((ev_cfg = read_file_or_else<std::string>(event_path, "")) == "")
        {
            attr_.type = -1;
            return;
        }

        /* Event configuration format:
         *   One or more terms with optional values, separated by ','.  (Taken from
         *   linux/Documentation/ABI/testing/sysfs-bus-event_source-devices-events):
         *
         *     <term>[=<value>][,<term>[=<value>]...]
         *
         *   Example (config for 'cpu/cache-misses' on an Intel Core i5-7200U):
         *
         *     event=0x2e,umask=0x41
         *
         *  */

        enum EVENT_CONFIG_REGEX_GROUPS
        {
            EC_WHOLE_MATCH,
            EC_TERM,
            EC_VALUE,
        };

        static const std::regex kv_regex(R"(([^=,]+)(?:=([^,]+))?)");

        Log::debug() << "parsing event configuration: " << ev_cfg;
        std::smatch kv_match;
        while (std::regex_search(ev_cfg, kv_match, kv_regex))
        {
            static const std::string default_value("0x1");

            const std::string& term = kv_match[EC_TERM];
            const std::string& value =
                (kv_match[EC_VALUE].length() != 0) ? kv_match[EC_VALUE] : default_value;

            std::string format;
            if ((format = read_file_or_else<std::string>(pmu_path / "format" / term, "")) == "")
            {
                attr_.type = -1;
                return;
            }

            std::uint64_t val = std::stol(value, nullptr, 0);
            Log::debug() << "parsing config assignment: " << term << " = " << std::hex
                         << std::showbase << val << std::dec << std::noshowbase;
            // Parse config terms //

            /* Format:  <term>:<bitmask>
             *
             * We only assign the terms 'config' and 'config1'.
             *
             * */

            static constexpr auto npos = std::string::npos;
            const auto colon = format.find_first_of(':');
            if (colon == npos)
            {
                attr_.type = -1;
                return;
            }

            const auto target_config = format.substr(0, colon);
            const auto mask = parse_bitmask(format.substr(colon + 1));

            if (target_config == "config")
            {
                attr_.config |= apply_mask(val, mask);
            }

            if (target_config == "config1")
            {
                attr_.config1 |= apply_mask(val, mask);
            }

            ev_cfg = kv_match.suffix();
        }

        Log::debug() << std::hex << std::showbase << "parsed event description: " << pmu_name << "/"
                     << event_name << "/type=" << attr_.type << ",config=" << attr_.config
                     << ",config1=" << attr_.config1 << std::dec << std::noshowbase << "/";

        scale_ = read_file_or_else<double>(event_path.replace_extension(".scale"), 1.0);
        unit_ = read_file_or_else<std::string>(event_path.replace_extension(".unit"), "#");
    }
};
} // namespace perf_cpp
