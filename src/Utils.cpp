#include "pch.h"

namespace utils
{
	bool is_number(const std::string& s)
	{
		return !s.empty() && std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
	}

    void MadeTextFromKeys(std::string& chr)
    {
        if(chr == "SPACE")
            chr = ' ';
        else if(chr == 'z')
            chr = 'y';
        else if(chr == 'Z')
            chr = 'Y';
        else if(chr == 'y')
            chr = 'z';
        else if(chr == 'Y')
            chr = 'Z';
    }

	std::string GetDataUnit(size_t input)
	{
		float fInput = static_cast<float>(input);

		if(fInput < 1024)
		{
			return fmt::format("{} B", (size_t)fInput);
		}

		fInput /= 1024;
		if(fInput < 1024)
		{
			return fmt::format("{:.2f} kB", fInput);
		}

		fInput /= 1024;
		if(fInput < 1024)
		{
			return fmt::format("{:.2f} MB", fInput);
		}

		fInput /= 1024;
		if(fInput < 1024)
		{
			return fmt::format("{:.2f} GB", fInput);
		}

		fInput /= 1024;
		if(fInput < 1024)
		{
			return fmt::format("{:.2f} TB", fInput);
		}

		return std::string("X");
	}
}