#include "reader.hpp"
#include "utils/debug.hpp"

// just for debuging, reset log file offset to read from starting
DEBUG_DECLARE(
    void Reader::reset_offset()
    {
        std::cout << "reseting the offset for debuging purpose!" << std::endl;
        offset = 0;
    }
);

// std::getline is not suitable for our case as the log files may be continuosly getting written by server,
// std::getline returns and stop writting line when it hits std::fstream::eof() or '\n' character
// when server is writting the logs before server finish writting line std::getline reads till last character and due to eof hit, returns
// and we get part of line
// which is incorrect in our case, we need full log line to parse
// custom_getline returns when it hits '\n' character and hence give full line
bool Reader::custom_getline(std::ifstream &ifile, std::string &line)
{
    std::streampos before_pos = ifile.tellg();
    char c = ' ';
    std::string buffer;

    while (ifile.get(c))
    {
        if (c == '\n')
        {
            line = buffer;
            return true;
        }
        buffer += c;
    }

    ifile.clear();
    ifile.seekg(before_pos);
    return false;
}

// incase our socket stops, we need to continue from last we ended, hence we maintain offset of log file in storage/.reader_offset
std::streampos Reader::read_offset() const
{
    std::ifstream i_state(STATE_FILE);
    long long raw_offset = 0;
    if (i_state) i_state >> raw_offset;
    else DEBUG_LOG("ERROR::READER:: cant open reader file! (line: " << __LINE__ << ")");

    std::streampos offset = static_cast<std::streampos>(raw_offset);

    return offset;
}

// saves the current offset into the storage/.reader_offset
std::streampos Reader::write_offset(std::streampos offset) const
{
    std::fstream o_state(STATE_FILE, std::ios::trunc | std::ios::out);
    o_state << offset;

    return offset;
}

Reader::Reader()
{
    offset = read_offset();
    status = true;
}

std::vector<std::string> Reader::read_logs(std::string file_name)
{
    std::ifstream ifile(file_name);
    if (!ifile)
    {
        // check the status of reader when you call read_logs
        status = false;
        return {};
    }

    ifile.seekg(offset);

    std::vector<std::string> logs;
    std::string line{};

    while (custom_getline(ifile, line))
    {
        logs.emplace_back(line);
        offset = ifile.tellg(); // capturing offset in loop because we want offset to be last readed line not -1 in case of eof
    }

    if (offset == -1) // ensure offset to be of last line not -1 (means eof), because log files updates continuously
    {
        ifile.clear();
        ifile.seekg(0, std::ios::end);
        offset = ifile.tellg();
    }

    write_offset(offset); // save offset

    return logs;
}