#include <patchers.h>
#include <logger.h>
#include "../ui/webui.h"

//success(filename, data)
std::vector<Patchers::Patcher*> patcherList;

jsfunction(Patchers::registerPatcher) {
	JSValueRef patcherId = JSUtils::GetUndefined();
	if (jsargc == 2) {
		JSObjectRef jsCallback = (JSObjectRef)jsargv[0];
		
		if (!jsCallback) {
			Logger::Print<Logger::WARNING>("Attemped to register a patcher with no callback");
			return patcherId;
		}
		if (JSValueIsUndefined(JSUtils::GetContext(), jsCallback)) {
			Logger::Print<Logger::WARNING>("Attemped to register a patcher, but the callback is undefined. Did the callback get GC'd?");
			return patcherId;
		}
		if (!JSValueIsObject(JSUtils::GetContext(), jsCallback)) {
			Logger::Print<Logger::WARNING>("Attemped to register a patcher, but the callback is not an object (and as a result cannot be a function). Did we get GC'd?");
			return patcherId;
		}

		std::string targetFile = JSUtils::GetString((JSStringRef)jsargv[1]);

		JSPatcher* jsPatcher = new JSPatcher(targetFile, jsCallback);
		
		size_t id = Patchers::RegisterPatcher(jsPatcher);
		patcherId = JSValueMakeNumber(JSUtils::GetContext(), id);
		return patcherId;
	}
	else {
		Logger::Print<Logger::FAILURE>("souped.registerPatcher called with the incorrect number of arguments");
	}
	return patcherId;
}

size_t Patchers::RegisterPatcher(Patcher* patcher) {
	patcherList.push_back(patcher);

	return patcherList.size() - 1;
};

void Patchers::DestroyPatcher(size_t idx) {
	patcherList.erase(patcherList.begin() + idx);
};

void Patchers::PatchData(std::string filename, std::string& data) {
	for (auto patcher : patcherList) {
		if (filename.find(patcher->selector) != std::string::npos) {
			//In case the patch fails, we want to fall back to the last successfull version of the data
			std::string prePatch = data;
			if (!patcher->DoPatchwork(filename, data)) {
				data = prePatch;
			}
		}
	}
}

Patchers::JSPatcher::JSPatcher(std::string selector, JSObjectRef jsCallback) : Patcher(selector)
{
	JSValueProtect(JSUtils::GetContext(), jsCallback);
	this->jsCallback = jsCallback;
}

Patchers::JSPatcher::~JSPatcher()
{
	JSValueUnprotect(JSUtils::GetContext(), jsCallback);
}

bool Patchers::Patcher::DoPatchwork(std::string, std::string&)
{
	return true;
}

bool Patchers::JSPatcher::DoPatchwork(std::string fileName, std::string& fileContent)
{
	//Acquire JS context from WebUI
	ultralight::RefPtr<ultralight::JSContext> ctxRef = WebUI::AcquireJSContext();
	JSUtils::SetContext(ctxRef->ctx());

	if (!jsCallback) {
		Logger::Print<Logger::WARNING>("Attemped to run a patcher with no callback");
		return false;
	}

	if (JSValueIsUndefined(JSUtils::GetContext(), jsCallback)) {
		Logger::Print<Logger::WARNING>("A patcher was executed but the callback is undefined. Did the callback get GC'd?");
		return false;
	}

	if (!JSValueIsObject(JSUtils::GetContext(), jsCallback)) {
		Logger::Print<Logger::WARNING>("A patcher was executed, but the callback is not an object (and as a result cannot be a function). Did we get GC'd?");
		return false;
	}

	if (this->selector.size() == 0) {
		Logger::Print<Logger::WARNING>("A javascript patcher was invoked with no target selector?");
		return false;
	}
	if (fileName.find(this->selector) == std::string::npos) {
		//The patch doesn't target this file
		return false;
	}
	std::string patchResult = "";
	bool success = false;

	JSStringRef jsFileName = JSUtils::CreateString(fileName);
	JSStringRef jsFileContent = JSUtils::CreateString(fileContent);

	const JSValueRef jsArgs[] = {
		(JSValueRef)JSValueMakeString(JSUtils::GetContext(), jsFileName),
		(JSValueRef)JSValueMakeString(JSUtils::GetContext(), jsFileContent)
	};

	//Call the function on the ui thread
	shared_thread& uiThread = WebUI::GetThread();
	JSValueRef result;
	uiThread.DoWork([&]() {
		result = JSUtils::CallFunction(jsCallback, 0, jsArgs, 2);
	});
	uiThread.AwaitCompletion();

	if (!JSValueIsObject(JSUtils::GetContext(), result)) {
		Logger::Print<Logger::WARNING>("JS Patcher callback did NOT return an object! Ignoring patch...");
		return false;
	}

	JSValueRef successful = JSUtils::ReadProperty((JSObjectRef)result, "successful");
	success = JSValueToBoolean(JSUtils::GetContext(), successful);
	if (success) {
		JSStringRef jsPatchedContent = (JSStringRef)JSUtils::ReadProperty((JSObjectRef)result, "data");
		patchResult = JSUtils::GetString(jsPatchedContent);
	}

	fileContent = patchResult;
	return success;
}
