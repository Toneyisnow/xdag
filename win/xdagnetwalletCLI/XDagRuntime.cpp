// xdagnetwalletCLI.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <msclr\marshal_cppstd.h>

#include "XDagRuntime.h"

#pragma unmanaged

#include "../xdag/xdag_runtime.h"


#pragma managed


using namespace System;
using namespace System::Runtime::InteropServices;
using namespace XDagNetWalletCLI;


XDagRuntime::XDagRuntime(IXDagWallet^ wallet)
{
	if (wallet == nullptr)
	{
		throw gcnew System::ArgumentNullException();
	}

	this->xdagWallet = wallet;

	// Set Logger Callback
	InputPasswordDelegate^ func = gcnew InputPasswordDelegate(this, &XDagRuntime::InputPassword);
	gch = GCHandle::Alloc(func);
	IntPtr funcPtr = Marshal::GetFunctionPointerForDelegate(func);
	InputPasswordStd necb = static_cast<InputPasswordStd>(funcPtr.ToPointer());
	xdag_set_password_callback_wrap(necb);

	ShowStateDelegate^ func2 = gcnew ShowStateDelegate(this, &XDagRuntime::ShowState);
	gch = GCHandle::Alloc(func2);
	IntPtr funcPtr2 = Marshal::GetFunctionPointerForDelegate(func2);
	g_xdag_show_state = static_cast<ShowStateStd>(funcPtr2.ToPointer());

}

XDagRuntime::~XDagRuntime()
{

}

void XDagRuntime::Start()
{
	std::string msg = "xdag.exe";
	char *argv[] = { (char*)msg.c_str() };

	xdag_init_wrap(1, argv, 1);
}

bool XDagRuntime::HasExistingAccount()
{
	return xdag_dnet_crpt_found();
}

void XDagRuntime::DoTesting()
{
}

int XDagRuntime::InputPassword(const char *prompt, char *buf, unsigned size)
{
	if (this->xdagWallet == nullptr)
	{
		return -1;
	}

	String ^ promptString = ConvertFromConstChar(prompt);

	String ^ passwordString = this->xdagWallet->OnPromptInputPassword(promptString, (UINT)size);

	const char* passwordChars = ConvertFromString(passwordString);

	if (strlen(passwordChars) == 0)
	{
		return -1;
	}

	strncpy_s(buf, size, passwordChars, size);

	return 0;
}

int XDagRuntime::ShowState(const char *state, const char *balance, const char *address)
{
	String^ stateString = ConvertFromConstChar(state);
	String^ balanceString = ConvertFromConstChar(balance);
	String^ addressString = ConvertFromConstChar(address);

	this->xdagWallet->OnUpdateState(stateString, balanceString, addressString);

	return 0;
}

String^ XDagRuntime::ConvertFromConstChar(const char* str)
{
	std::string promptStd(str);
	return msclr::interop::marshal_as<System::String^>(promptStd);
}

const char* XDagRuntime::ConvertFromString(String^ str)
{
	std::string stdString = msclr::interop::marshal_as<std::string>(str);
	return stdString.c_str();
}