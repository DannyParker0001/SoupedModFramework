#include <patchers.h>

//success(filename, data)
std::vector<std::function<bool(std::string, std::string&)>> patcherList;

size_t Patchers::RegisterPatcher(std::function<bool(std::string, std::string&)> patcher) {
	patcherList.push_back(patcher);
	return patcherList.size() - 1;
};

void Patchers::DestroyPatcher(size_t idx) {
	patcherList.erase(patcherList.begin() + idx);
};

void Patchers::PatchData(std::string filename, std::string& data) {
	for (auto patcher : patcherList) {
		//In case the patch fails, we want to fall back to the last successfull version of the data
		std::string prePatch = data;
		if (!patcher(filename, data)) {
			data = prePatch;
		}
	}
}