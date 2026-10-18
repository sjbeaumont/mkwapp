#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;

std::string capitalize(std::string str) {
	if (!str.empty()) {
		str[0] = std::toupper(str[0]);
	}
	return str;
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cerr << "usage: mkwapp <url>\n";
		return 1;
	}

	if (strcmp(argv[1], "-r") == 0 && argc > 2) {
		const char* homeenv = std::getenv("HOME");
		fs::path homedir(homeenv);

		std::string rmname = argv[2];
		fs::path entry = homedir / ".local" / "share" / "applications" / (rmname + ".desktop");
		
		if (fs::exists(entry)) {
			if (fs::remove(entry)) { std::cout << "done\n"; return 0; }
			else { std::cout << "couldnt remove app\n"; return 1; }
		}
		else {
			std::cout << "file not found\n";
			return 1;
		}
	}

	std::string tarurl = argv[1];
	std::string host = tarurl;

	size_t protopos = host.find("://");
	if (protopos != std::string::npos) {
		host = host.substr(protopos + 3);
	} else {
		tarurl = "https://" + tarurl;
	}

	size_t slashpos = host.find('/');
	if (slashpos != std::string::npos) {
		host = host.substr(0, slashpos);
	}

	if (host.rfind("www.", 0) == 0) {
		host = host.substr(4);
	}

	size_t dotpos = host.find('.');
	if (dotpos == 0 || host.empty()) {
		std::cerr << "couldnt find app name\n";
		return 1;
	}

	std::string cleanname = (dotpos == std::string::npos) ? host : host.substr(0, dotpos);

	std::string  appname = capitalize(cleanname);

	std::string filebase = cleanname;
	std::transform(filebase.begin(), filebase.end(), filebase.begin(), ::tolower);
	std::string filename = filebase + ".desktop";

	const char* homeenv = std::getenv("HOME");
	if (!homeenv) {
		std::cerr << "couldnt find home dir\n";
		return 1;
	}

	fs::path homedir(homeenv);
	fs::path launcherdir = homedir / ".local" / "share" / "applications";
	fs::path launcherpath = launcherdir / filename;

	try {
		fs::create_directories(launcherdir);
	} catch (const fs::filesystem_error& e) {
		std::cerr << "err creating dir: " << e.what() << "\n";
		return 1;
	}

	std::ofstream outfile(launcherpath);
	if (!outfile) {
		std::cerr << "couldnt write desktipe file\n";
		return 1;
	}

	outfile << "[Desktop Entry]\n"
			<< "Version=1.0\n"
			<< "Type=Application\n"
			<< "Name=" << appname << "\n"
			<< "Comment=Web app for " << appname << "\n"
			<< "Exec=chromium --enable-features=UseOzonePlatform --ozone-platform=wayland --app=" << tarurl << "\n"
			<< "Icon=web-browser\n"
			<< "Terminal=false\n"
			<< "Categories=Network;WebBrowser;\n";

	outfile.close();

	try {
		fs::permissions(launcherpath, fs::perms::owner_all | fs::perms::group_read | fs::perms::others_read);
	} catch (const fs::filesystem_error& e) {
		std::cerr << "couldnt set executable perms: " << e.what() << "\n";
	}

	std::cout << "done\n";
	return 0;
}