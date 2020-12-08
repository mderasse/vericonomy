#ifndef BITCOIN_DOWNLOADER_H
#define BITCOIN_DOWNLOADER_H

#include <string>

#if defined(__arm__) || defined(__aarch64__)
const std::string BOOTSTRAP_VRM_URL("https://files.vericonomy.com/vrm/bootstrap-arm/bootstrap.zip");
const std::string BOOTSTRAP_VRC_URL("https://files.vericonomy.com/vrc/bootstrap-arm/bootstrap.zip");
#else
const std::string BOOTSTRAP_VRM_URL("https://files.vericonomy.com/vrm/bootstrap/bootstrap.zip");
const std::string BOOTSTRAP_VRC_URL("https://files.vericonomy.com/vrc/bootstrap/bootstrap.zip");
#endif

const std::string VERSIONFILE_VRM_URL("https://files.vericonomy.com/vrm/VERSION_VRM.json");
const std::string CLIENT_VRM_URL("https://files.vericonomy.com/vrm/");

const std::string VERSIONFILE_VRC_URL("https://files.vericonomy.com/vrc/VERSION_VRC.json");
const std::string CLIENT_VRC_URL("https://files.vericonomy.com/vrc/");

void downloadBootstrap();
void applyBootstrap();
void downloadVersionFile();
void downloadClient(std::string fileName);
int getArchitecture();

#endif // BITCOIN_DOWNLOADER_H