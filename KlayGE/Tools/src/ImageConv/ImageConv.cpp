#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/filesystem.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Math.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Texture.hpp>

#include <iostream>
#include <vector>
#include <string>

#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations" // Ignore auto_ptr declaration
#endif
#include <boost/program_options.hpp>
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic pop
#endif

#include <FreeImage.h>

using namespace std;
using namespace KlayGE;

namespace
{
	bool ConvertImage(std::string const & input_name, std::string const & output_name)
	{
		KFL_UNUSED(output_name);

		FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(input_name.c_str(), 0);
		if (fif == FIF_UNKNOWN) 
		{
			fif = FreeImage_GetFIFFromFilename(input_name.c_str());
		}
		if (fif == FIF_UNKNOWN)
		{
			return false;
		}

		FIBITMAP* dib = nullptr;
		if (FreeImage_FIFSupportsReading(fif))
		{
			dib = FreeImage_Load(fif, input_name.c_str());
		}
		if (!dib)
		{
			return false;
		}

		uint8_t* const bits = FreeImage_GetBits(dib);
		uint32_t const width = FreeImage_GetWidth(dib);
		uint32_t const height = FreeImage_GetHeight(dib);
		if ((bits == nullptr) || (width == 0) || (height == 0))
		{
			return false;
		}

		uint32_t const bpp = FreeImage_GetBPP(dib);
		uint32_t const r_mask = FreeImage_GetRedMask(dib);
		uint32_t const g_mask = FreeImage_GetGreenMask(dib);
		uint32_t const b_mask = FreeImage_GetBlueMask(dib);
		ElementFormat format = EF_ABGR8;
		switch (bpp)
		{
		case 16:
			if ((r_mask == (0x1F << 10)) && (g_mask == (0x1F << 5)) && (b_mask == 0x1F))
			{
				format = EF_A1RGB5;
			}
			else if ((r_mask == (0x1F << 11)) && (g_mask == (0x3F << 5)) && (b_mask == 0x1F))
			{
				format = EF_R5G6B5;
			}
			break;

		case 24:
			if ((r_mask == 0xFF0000) && (g_mask == 0xFF00) && (b_mask == 0xFF))
			{
				format = EF_ARGB8;
			}
			else if ((r_mask == 0xFF) && (g_mask == 0xFF00) && (b_mask == 0xFF0000))
			{
				format = EF_ABGR8;
			}
			break;

		case 32:
			if ((r_mask == 0xFF0000) && (g_mask == 0xFF00) && (b_mask == 0xFF))
			{
				format = EF_ARGB8;
			}
			else if ((r_mask == 0xFF) && (g_mask == 0xFF00) && (b_mask == 0xFF0000))
			{
				format = EF_ABGR8;
			}
			break;

		default:
			KFL_UNREACHABLE("Unknown format.");
		}

		ElementInitData init_data;
		init_data.data = dib;
		init_data.row_pitch = FreeImage_GetPitch(dib);
		init_data.slice_pitch = height * init_data.row_pitch;
		SaveTexture(output_name, Texture::TT_2D, width, height, 1, 1, 1, format, init_data);
	
		FreeImage_Unload(dib);

		return true;
	}
}

int main(int argc, char* argv[])
{
	std::string input_name;
	std::string output_name;
	bool quiet = false;

	boost::program_options::options_description desc("Allowed options");
	desc.add_options()
		("help,H", "Produce help message")
		("input-path,I", boost::program_options::value<std::string>(), "Input image path.")
		("output-path,O", boost::program_options::value<std::string>(), "Output image path.")
		("quiet,q", boost::program_options::value<bool>()->implicit_value(true), "Quiet mode.")
		("version,v", "Version.");

	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
	boost::program_options::notify(vm);

	if ((argc <= 1) || (vm.count("help") > 0))
	{
		cout << desc << endl;
		return 1;
	}
	if (vm.count("version") > 0)
	{
		cout << "KlayGE Image Converter, Version 1.0.0" << endl;
		return 1;
	}
	if (vm.count("input-path") > 0)
	{
		input_name = vm["input-path"].as<std::string>();
	}
	else
	{
		cout << "Need input image path." << endl;
		return 1;
	}
	if (vm.count("output-path") > 0)
	{
		output_name = vm["output-path"].as<std::string>();
	}
	if (vm.count("quiet") > 0)
	{
		quiet = vm["quiet"].as<bool>();
	}

	std::string file_name = ResLoader::Instance().Locate(input_name);
	if (file_name.empty())
	{
		cout << "Could NOT find " << input_name << endl;
		Context::Destroy();
		return 1;
	}

	if (output_name.empty())
	{
		filesystem::path input_path(file_name);
		output_name = (input_path.parent_path() / input_path.stem()).string();
		if (input_path.extension() == "dds")
		{
			output_name += "_converted";
		}
		output_name += ".dds";
	}

	bool succ = ConvertImage(file_name, output_name);

	if (succ && !quiet)
	{
		cout << "MeshML has been saved to " << output_name << "." << endl;
	}

	Context::Destroy();

	return 0;
}
