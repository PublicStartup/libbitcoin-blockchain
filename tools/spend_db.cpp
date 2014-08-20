#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/blockchain.hpp>
using namespace bc;
using namespace bc::chain;

void show_help()
{
    std::cout << "Usage: spend_db COMMAND FILE [ARGS]" << std::endl;
    std::cout << std::endl;
    std::cout << "The most commonly used spend_db commands are:" << std::endl;
    std::cout << "  initialize_new  "
        << "Create a new history_database" << std::endl;
    std::cout << "  get             "
        << "Fetch spend by outpoint" << std::endl;
    std::cout << "  store           "
        << "Store a spend" << std::endl;
    std::cout << "  remove          "
        << "Remove a spend" << std::endl;
    std::cout << "  help            "
        << "Show help for commands" << std::endl;
}

void show_command_help(const std::string& command)
{
    if (command == "initialize_new")
    {
        std::cout << "Usage: spend_db " << command << " FILE "
            << "" << std::endl;
    }
    else if (command == "get")
    {
        std::cout << "Usage: spend_db " << command << " FILE "
            << "OUTPOINT" << std::endl;
    }
    else if (command == "store")
    {
        std::cout << "Usage: spend_db " << command << " FILE "
            << "OUTPOINT SPEND" << std::endl;
    }
    else if (command == "remove")
    {
        std::cout << "Usage: spend_db " << command << " FILE "
            << "OUTPOINT" << std::endl;
    }
    else
    {
        std::cout << "No help available for " << command << std::endl;
    }
}

template <typename Point>
bool parse_point(Point& point, const std::string& arg)
{
    std::vector<std::string> strs;
    boost::split(strs, arg, boost::is_any_of(":"));
    if (strs.size() != 2)
    {
        std::cerr << "spend_db: bad point provided." << std::endl;
        return false;
    }
    const std::string& hex_string = strs[0];
    if (hex_string.size() != 64)
    {
        std::cerr << "spend_db: bad point provided." << std::endl;
        return false;
    }
    point.hash = decode_hash(hex_string);
    const std::string& index_string = strs[1];
    try
    {
        point.index = boost::lexical_cast<uint32_t>(index_string);
    }
    catch (const boost::bad_lexical_cast&)
    {
        std::cerr << "spend_db: bad point provided." << std::endl;
        return false;
    }
    return true;
}

bool parse_key(short_hash& key, const std::string& arg)
{
    payment_address payaddr;
    if (!payaddr.set_encoded(arg))
    {
        std::cerr << "spend_db: bad KEY." << std::endl;
        return -1;
    }
    key = payaddr.hash();
    return true;
}

template <typename Uint>
bool parse_uint(Uint& value, const std::string& arg)
{
    try
    {
        value = boost::lexical_cast<Uint>(arg);
    }
    catch (const boost::bad_lexical_cast&)
    {
        std::cerr << "spend_db: bad value provided." << std::endl;
        return false;
    }
    return true;
}

int main(int argc, char** argv)
{
    typedef std::vector<std::string> string_list;
    if (argc < 2)
    {
        show_help();
        return -1;
    }
    const std::string command = argv[1];
    if (command == "help" || command == "-h" || command == "--help")
    {
        if (argc == 3)
        {
            show_command_help(argv[2]);
            return 0;
        }
        show_help();
        return 0;
    }
    if (argc < 3)
    {
        show_command_help(command);
        return -1;
    }
    const std::string filename = argv[2];
    string_list args;
    for (size_t i = 3; i < argc; ++i)
        args.push_back(argv[i]);
    if (command == "initialize_new")
    {
        touch_file(filename);
    }
    spend_database db(filename);
    if (command == "initialize_new")
    {
        db.initialize_new();
        return 0;
    }
    else if (command == "get")
    {
        if (args.size() != 1)
        {
            show_command_help(command);
            return -1;
        }
        output_point outpoint;
        if (!parse_point(outpoint, args[0]))
            return -1;
        db.start();
        spend_result result = db.get(outpoint);
        if (!result)
        {
            std::cout << "Not found!" << std::endl;
            return -1;
        }
        std::cout << result.hash() << ":" << result.index() << std::endl;
    }
    else if (command == "store")
    {
        if (args.size() != 2)
        {
            show_command_help(command);
            return -1;
        }
        output_point outpoint;
        if (!parse_point(outpoint, args[0]))
            return -1;
        input_point spend;
        if (!parse_point(spend, args[1]))
            return -1;
        db.start();
        db.store(outpoint, spend);
        db.sync();
    }
    else if (command == "remove")
    {
        if (args.size() != 1)
        {
            show_command_help(command);
            return -1;
        }
        output_point outpoint;
        if (!parse_point(outpoint, args[0]))
            return -1;
        db.start();
        db.remove(outpoint);
        db.sync();
    }
/*
    else if (command == "add_row")
    {
        if (args.size() != 4)
        {
            show_command_help(command);
            return -1;
        }
        short_hash key;
        if (!parse_key(key, args[0]))
            return -1;
        output_point outpoint;
        if (!parse_point(outpoint, args[1]))
            return -1;
        uint32_t output_height;
        if (!parse_uint(output_height, args[2]))
            return -1;
        uint64_t value;
        if (!parse_uint(value, args[3]))
            return -1;
        db.start();
        db.add_row(key, outpoint, output_height, value);
        db.sync();
        return 0;
    }
    else if (command == "add_spend")
    {
        if (args.size() != 4)
        {
            show_command_help(command);
            return -1;
        }
        short_hash key;
        if (!parse_key(key, args[0]))
            return -1;
        output_point previous;
        if (!parse_point(previous, args[1]))
            return -1;
        input_point spend;
        if (!parse_point(spend, args[2]))
            return -1;
        uint32_t spend_height;
        if (!parse_uint(spend_height, args[3]))
            return -1;
        db.start();
        db.add_spend(key, previous, spend, spend_height);
        return 0;
    }
    else if (command == "delete_spend")
    {
        if (args.size() != 2)
        {
            show_command_help(command);
            return -1;
        }
        short_hash key;
        if (!parse_key(key, args[0]))
            return -1;
        input_point spend;
        if (!parse_point(spend, args[1]))
            return -1;
        db.start();
        db.delete_spend(key, spend);
        return 0;
    }
    else if (command == "delete_last_row")
    {
        if (args.size() != 1)
        {
            show_command_help(command);
            return -1;
        }
        short_hash key;
        if (!parse_key(key, args[0]))
            return -1;
        db.start();
        db.delete_last_row(key);
        db.sync();
        return 0;
    }
    else if (command == "fetch")
    {
        if (args.size() < 1 || args.size() > 3)
        {
            show_command_help(command);
            return -1;
        }
        short_hash key;
        if (!parse_key(key, args[0]))
            return -1;
        size_t limit = 0;
        if (args.size() >= 2)
            if (!parse_uint(limit, args[1]))
                return -1;
        index_type start = 0;
        if (args.size() >= 3)
            if (!parse_uint(start, args[2]))
                return -1;
        auto fetch = [](const std::error_code& ec,
            const blockchain::history_list& history,
            const index_type stop)
        {
            if (ec)
            {
                std::cout << "Error: " << ec.message() << std::endl;
                return;
            }
            for (const auto& row: history)
            {
                std::cout << row.output.hash << ":" << row.output.index
                    << " " << row.output_height << " " << row.value;
                if (row.spend_height)
                    std::cout << " " << row.spend.hash << ":"
                        << row.spend.index << " " << row.spend_height;
                std::cout << std::endl;
            }
        };
        db.start();
        db.fetch(key, fetch, limit, start);
        return 0;
    }
*/
    else
    {
        std::cout << "spend_db: '" << command
            << "' is not a spend_db command. "
            << "See 'spend_db --help'." << std::endl;
        return -1;
    }
    return 0;
}
