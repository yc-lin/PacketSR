// PacketSRDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PacketSR.h"
#include "PacketSRDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPacketSRDlg dialog




CPacketSRDlg::CPacketSRDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPacketSRDlg::IDD, pParent)
    , port_number(0)
    , stuff_packet_size(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    port_number = 1234;
    local_socket = INVALID_SOCKET;
    stop_send_packet = true;
    stop_recv_packet = true;
    elasped_time = 0;
    sended_packet_numbers = 0;
    recved_packet_numbers = 0;
    recved_total_sending_packet_numbers = 0;
}

CPacketSRDlg::~CPacketSRDlg()
{
    WSACleanup();
}

void CPacketSRDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STA_SENDPACKET_N2, sta_sended_packet_number_);
    DDX_Control(pDX, IDC_IPADDRESS_TO, ipc_remote_address);
    DDX_Control(pDX, IDC_COMBO1, ccbox_nic_list);
    DDX_Control(pDX, IDC_STA_AVG_RECEIVE_PACKET_NUMBER, avg_packet_one_seconds);
    DDX_Control(pDX, ID_START, btn_start);
    DDX_Control(pDX, ID_STOP, btn_stop);
    DDX_Text(pDX, IDC_EDIT_PACKET_SIZE, stuff_packet_size);
	DDV_MinMaxInt(pDX, stuff_packet_size, 0, 1450);
}

BEGIN_MESSAGE_MAP(CPacketSRDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(ID_START, &CPacketSRDlg::OnBnClickedStart)
    ON_BN_CLICKED(ID_STOP, &CPacketSRDlg::OnBnClickedStop)
    ON_BN_CLICKED(IDC_RADIO_BROADCAST, &CPacketSRDlg::OnBnClickedRadioBroadcast)
    ON_BN_CLICKED(IDC_RADIO_UNICAST, &CPacketSRDlg::OnBnClickedRadioUnicast)
    ON_WM_CLOSE()
    ON_CBN_SELCHANGE(IDC_COMBO1, &CPacketSRDlg::OnCbnSelchangeCombo1)
    ON_BN_CLICKED(IDC_RADIO_SEND_PACKET, &CPacketSRDlg::OnBnClickedRadioSendPacket)
    ON_BN_CLICKED(IDC_RADIO_RECV_PACKET, &CPacketSRDlg::OnBnClickedRadioRecvPacket)
END_MESSAGE_MAP()


// CPacketSRDlg message handlers

BOOL CPacketSRDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	ShowWindow(SW_MINIMIZE);

	// TODO: Add extra initialization here
    int retval = 0;
    if ((retval = WSAStartup(0x202, &wsaData)) != 0)
    {
        WSACleanup();
        return -1;
    }

    CButton *broadcst_btn_ptr = static_cast<CButton*>(GetDlgItem(IDC_RADIO_BROADCAST));
    broadcst_btn_ptr->SetCheck(BST_CHECKED);

    CButton *send_btn_ptr = static_cast<CButton*>(GetDlgItem(IDC_RADIO_SEND_PACKET));
    send_btn_ptr->SetCheck(BST_CHECKED);
    CButton *recv_btn_ptr = static_cast<CButton*>(GetDlgItem(IDC_RADIO_RECV_PACKET));
    recv_btn_ptr->SetCheck(BST_UNCHECKED);

    use_broadcast = TRUE;
    is_client = TRUE;

    ipc_remote_address.SetAddress(0, 0, 0, 0);

    CEdit *edit_port_ptr = static_cast<CEdit*>(GetDlgItem(IDC_EDIT_PORT));
    edit_port_ptr->SetWindowText("1234");

    edit_port_ptr = static_cast<CEdit*>(GetDlgItem(IDC_EDIT_PACKET_SIZE));
    edit_port_ptr->SetWindowText("0");

    detect_ip_list();

    ShowWindow(SW_RESTORE);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPacketSRDlg::detect_ip_list()
{
    ULONG	          ulOutBufLen	     = sizeof(IP_ADAPTER_INFO);
    PIP_ADAPTER_INFO  PIPAdapterInfo     = NULL;
    PIP_ADAPTER_INFO  PIPAdapterInfoHead = NULL;
    std::set<CString> NICsIPList;

    PIPAdapterInfo = (IP_ADAPTER_INFO *) malloc( sizeof(IP_ADAPTER_INFO));

    // First time, we must got ERROR_BUFFER_OVERFLOW, so we need get right buffer size
    if(GetAdaptersInfo(PIPAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
    {
        free(PIPAdapterInfo);
        // Get right PIP_ADAPTER_INFO buffer size...
        PIPAdapterInfo = (IP_ADAPTER_INFO *) malloc (ulOutBufLen);	
    }

    DWORD dwRet = GetAdaptersInfo(PIPAdapterInfo, &ulOutBufLen);
    if(dwRet == ERROR_SUCCESS)
    {
        PIPAdapterInfoHead = PIPAdapterInfo; 
        while(PIPAdapterInfo)
        {
            UINT unIP = 0;
            IP_ADDR_STRING* pIPADDRSTR = NULL;

            pIPADDRSTR = &(PIPAdapterInfo->IpAddressList);
            while(pIPADDRSTR) {
                CString csIPtmp(reinterpret_cast<char*>(&pIPADDRSTR->IpAddress));
                //unIP = htonl(inet_addr((char*)&(pIPADDRSTR->IpAddress)));
                // Check IP is not 0 and IP location doesn't exist in IP list
                if(!csIPtmp.IsEmpty()){
                    if(NICsIPList.find(csIPtmp) == NICsIPList.end())
                    {
                        // Create IP list				
                        NICsIPList.insert(csIPtmp);
                    }
                }
                // Find next IP information
                pIPADDRSTR  =   pIPADDRSTR->Next;
            }	
            // Get next NIC information
            PIPAdapterInfo = PIPAdapterInfo->Next;
        }
    } else {

        TRACE("[UDP] DetectNICnumber:GetAdaptersInfo(), ERROR CODE %d", dwRet);
    }
    // delete NIC info
    if(PIPAdapterInfoHead)
    {
        free(PIPAdapterInfoHead);
        PIPAdapterInfoHead = NULL;
    }


    if(!NICsIPList.empty())
    {
        int index = 0;
        for(std::set<CString>::iterator it = NICsIPList.begin();
            it != NICsIPList.end(); ++it)
        {
            ccbox_nic_list.AddString(*it);
        }

        ccbox_nic_list.SetCurSel(0);
    }
    OnCbnSelchangeCombo1();
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPacketSRDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPacketSRDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CPacketSRDlg::OnCancel()
{
}

void CPacketSRDlg::OnClose()
{
    CDialog::OnCancel();
}

void CPacketSRDlg::OnBnClickedStart()
{
    // reset socket
    if(local_socket != INVALID_SOCKET)
    {
        memset((char*)&remote_addr,0,sizeof(remote_addr));
        if(shutdown(local_socket , SD_BOTH) == SOCKET_ERROR )
        {
            TRACE("shutdown Server socket, ERROR CODE = %d", WSAGetLastError());
        }
        closesocket(local_socket);
        local_socket = INVALID_SOCKET;

        sended_packet_numbers = 0;
        recved_packet_numbers = 0;
        recved_total_sending_packet_numbers = 0;
        update_sended_packet_number();
        update_recved_packet_number();
    }

    // get server port
    CEdit *edit_port_ptr = static_cast<CEdit*>(GetDlgItem(IDC_EDIT_PORT));
    CString cs_value;
    edit_port_ptr->GetWindowText(cs_value);
    port_number = _ttoi(cs_value.GetString());

    // retrieve packet size
    edit_port_ptr = static_cast<CEdit*>(GetDlgItem(IDC_EDIT_PACKET_SIZE));
    edit_port_ptr->GetWindowText(cs_value);
    stuff_packet_size = _ttoi(cs_value.GetString());

    // set remote address
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port   = htons ((u_short)port_number);

    if (!use_broadcast) {
        // get Address
        BYTE	nField0 = 0;
        BYTE	nField1 = 0;
        BYTE	nField2 = 0;
        BYTE	nField3 = 0;

        ipc_remote_address.GetAddress(nField0,nField1,nField2,nField3);

        unsigned int un_ip = nField0 * 0x1000000 + nField1 * 0x10000 + nField2 * 0x100 + nField3;

        char pch_select_server_ip[16] = {0x0};
        sprintf_s(pch_select_server_ip,
            "%u.%u.%u.%u",
            (un_ip >> 24),
            (un_ip >> 16) % 256,
            (un_ip >>  8) % 256,
            (un_ip % 256));

        remote_addr.sin_addr.s_addr = inet_addr(pch_select_server_ip);
        if (remote_addr.sin_addr.s_addr == INADDR_NONE){
            TRACE("bad ip address %s\n");
            return; 
        }
    } else {
        remote_addr.sin_addr.s_addr  = INADDR_BROADCAST;
    }

    int len = sizeof(struct sockaddr);

    if (local_socket < 0){
        TRACE("socket creating failed\n");
        return;
    }

    // set local address and socket

    SOCKADDR_IN local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons((u_short)port_number);
    
    if (is_client) {
        local_addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        CString cs_bind_ip;
        int nSelectIP = ccbox_nic_list.GetCurSel();
        if(nSelectIP != -1) {
            ccbox_nic_list.GetLBText(nSelectIP , cs_bind_ip);
            unsigned int temp_ip = inet_addr((char*)(cs_bind_ip.GetString()));
            local_addr.sin_addr.s_addr = temp_ip;
        } else {
            TRACE("Set Bind IP error\n");
            return;
        } 
    }

    local_socket = socket(AF_INET,SOCK_DGRAM, IPPROTO_UDP);
    const int nOff = 0;
    if (setsockopt(local_socket,SOL_SOCKET,SO_REUSEADDR,(char*)&nOff,sizeof(nOff)) == SOCKET_ERROR)
    {
        TRACE("CreateSocket_SO_REUSEADDR_Error = %d\n", WSAGetLastError());
    }
    
    if (use_broadcast) {
        if(setsockopt(local_socket,SOL_SOCKET,SO_BROADCAST,(LPSTR)&use_broadcast,sizeof(BOOL)) == SOCKET_ERROR)
        {
            TRACE("Set Broadcast bind port = %d fail, ERROR CODE = %d\n", local_socket, WSAGetLastError());
            return;
        }
    }

    
    int nRet = SOCKET_ERROR;
    nRet = bind(local_socket,(SOCKADDR *)&local_addr,sizeof(SOCKADDR_IN));
    if( nRet  == SOCKET_ERROR) {
        TRACE("bind failed\n");
        return;
    }
    if (is_client) {
        TRACE("Sending...\n");
        stop_send_packet = false;
        AfxBeginThread(create_send_packet, reinterpret_cast<LPVOID>(this), THREAD_PRIORITY_NORMAL);
    } else {
        TRACE("Recving...\n");
        stop_recv_packet = false;
        AfxBeginThread(create_recv_packet, reinterpret_cast<LPVOID>(this), THREAD_PRIORITY_NORMAL);
    }

    btn_start.EnableWindow(FALSE);
    btn_stop.EnableWindow(TRUE);
}

void CPacketSRDlg::OnBnClickedStop()
{
    stop_send_packet = true;
    stop_recv_packet = true;
    if (!is_client) {
        closesocket(local_socket);
        local_socket = INVALID_SOCKET;
    }

    PostMessage(WM_INC_RECV_PACKET);
    PostMessage(WM_INC_RECV_PACKET_RESULT);
    btn_start.EnableWindow(TRUE);
    btn_stop.EnableWindow(FALSE);
    TRACE("Stop...\n");
}



LRESULT CPacketSRDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    // TODO: Add your specialized code here and/or call the base class
    switch (message) {
        case WM_INC_SEND_PACKET:
            update_sended_packet_number();
            break;
        case WM_INC_RECV_PACKET:
            update_recved_packet_number();
            break;
        case WM_INC_RECV_PACKET_RESULT:
            update_recved_packet_result();
            break;
        default:
            return CDialog::WindowProc(message, wParam, lParam);
    }
    return CDialog::WindowProc(message, wParam, lParam);
}


void CPacketSRDlg::OnCbnSelchangeCombo1()
{
        int nSelectIP = ccbox_nic_list.GetCurSel();
        if(nSelectIP != -1)
            ccbox_nic_list.GetLBText(nSelectIP , cs_nic_ip);
}

void CPacketSRDlg::OnBnClickedRadioBroadcast()
{
    CButton *broadcst_btn_ptr = static_cast<CButton*>(GetDlgItem(IDC_RADIO_BROADCAST));
    CButton *unciast_btn_ptr = static_cast<CButton*>(GetDlgItem(IDC_RADIO_UNICAST));

    if (broadcst_btn_ptr->GetCheck() == BST_CHECKED) {
        use_broadcast = TRUE;
        unciast_btn_ptr->SetCheck(BST_UNCHECKED);
    } else {
        use_broadcast = FALSE;
        unciast_btn_ptr->SetCheck(BST_CHECKED);
    }
}

void CPacketSRDlg::OnBnClickedRadioUnicast()
{
    CButton *broadcst_btn_ptr = static_cast<CButton*>(GetDlgItem(IDC_RADIO_BROADCAST));
    CButton *unciast_btn_ptr = static_cast<CButton*>(GetDlgItem(IDC_RADIO_UNICAST));
    if (unciast_btn_ptr->GetCheck() == BST_CHECKED) {
        use_broadcast = FALSE;
        broadcst_btn_ptr->SetCheck(BST_UNCHECKED);
    } else {
        use_broadcast = TRUE;
        broadcst_btn_ptr->SetCheck(BST_CHECKED);
    }
}

void CPacketSRDlg::OnBnClickedRadioSendPacket()
{
    CButton *send_btn_ptr = static_cast<CButton*>(GetDlgItem(IDC_RADIO_SEND_PACKET));
    CButton *recv_btn_ptr = static_cast<CButton*>(GetDlgItem(IDC_RADIO_RECV_PACKET));

    if (send_btn_ptr->GetCheck() == BST_CHECKED) {
        is_client = TRUE;
        recv_btn_ptr->SetCheck(BST_UNCHECKED);
    } else {
        is_client = FALSE;
        recv_btn_ptr->SetCheck(BST_CHECKED);
    }
}

void CPacketSRDlg::OnBnClickedRadioRecvPacket()
{
    CButton *send_btn_ptr = static_cast<CButton*>(GetDlgItem(IDC_RADIO_SEND_PACKET));
    CButton *recv_btn_ptr = static_cast<CButton*>(GetDlgItem(IDC_RADIO_RECV_PACKET));
    if (recv_btn_ptr->GetCheck() == BST_CHECKED) {
        is_client = FALSE;
        send_btn_ptr->SetCheck(BST_UNCHECKED);
        ipc_remote_address.EnableWindow(TRUE);
        //ipc_from_address.EnableWindow(TRUE);



    } else {
        is_client = TRUE;
        send_btn_ptr->SetCheck(BST_CHECKED);
    }
}

UINT CPacketSRDlg::create_send_packet( void *void_ptr )
{
    CPacketSRDlg *dlg = (CPacketSRDlg *)void_ptr;

    size_t len = sizeof(struct sockaddr);
    DWORD ticket_count = GetTickCount();
    SRPACKET command_packet;
    command_packet.maincommand = 0x01;
    command_packet.subcommand  = SR_PACKET_SUB_COMMAND_START;
    
    int buffer_size = sizeof(SRPACKET_END) + dlg->stuff_packet_size;
    char *buffer_ptr = new char[buffer_size];
    memset(buffer_ptr, 0x00, buffer_size);

    memcpy(buffer_ptr, &command_packet, sizeof(SRPACKET));
    int send_size = 0;
    // send start command
    for (int i=0;i <5; ++i) {
        send_size = sendto(dlg->local_socket,
            buffer_ptr,
            buffer_size,
            0,
            (struct sockaddr*) &(dlg->remote_addr),
            sizeof(sockaddr_in));
    }
    // delay send test packet
    Sleep(SR_DELAY_STAR_COMMAND);


    command_packet.subcommand  = SR_PACKET_SUB_COMMAND_SENDING;
    memcpy(buffer_ptr, &command_packet, sizeof(SRPACKET));
    while(!dlg->stop_send_packet)
    {
        send_size = sendto(dlg->local_socket,
            buffer_ptr,
            buffer_size,
            0,
            (struct sockaddr*) &(dlg->remote_addr),
            sizeof(sockaddr_in));
        if(send_size)
            ++dlg->sended_packet_numbers;

        //Sleep(100);
        if ((GetTickCount() - ticket_count) >= 100 ) {
            dlg->PostMessage(WM_INC_SEND_PACKET);
            ticket_count = GetTickCount();
        }
    }

    SRPACKET_END packet_end;
    packet_end.command.maincommand  = 0x01;
    packet_end.command.subcommand   = SR_PACKET_SUB_COMMAND_END;
    packet_end.sended_packet_number = dlg->sended_packet_numbers;
    memcpy(buffer_ptr, &packet_end, sizeof(SRPACKET_END));
    // send end command
    for (int i=0;i <5; ++i) {
        send_size = sendto(dlg->local_socket,
            buffer_ptr,
            buffer_size,
            0,
            (struct sockaddr*) &(dlg->remote_addr),
            sizeof(sockaddr_in));
    }
    delete buffer_ptr;

    dlg->OnBnClickedStop();
    dlg->PostMessage(WM_INC_SEND_PACKET);
    AfxEndThread(0, TRUE);
    return 0;
}

void CPacketSRDlg::update_sended_packet_number()
{
    CStatic *sta_packet_number_ptr = static_cast<CStatic*>(GetDlgItem(IDC_STA_SENDPACKET_N2));
    CString cs_packet_number;
    cs_packet_number.Format("%lu", sended_packet_numbers);
    sta_packet_number_ptr->SetWindowText(cs_packet_number);
}

UINT CPacketSRDlg::create_recv_packet( void *void_ptr )
{
    CPacketSRDlg *dlg = (CPacketSRDlg *)void_ptr;

    DWORD msg_length  = sizeof(SRPACKET_END) + dlg->stuff_packet_size;
    char  *msg_ptr = new char [msg_length];
    DWORD recv_msg_leng = 0;
    SOCKADDR_IN	addr_from; 
    int     addr_length = sizeof(SOCKADDR_IN);
    DWORD ticket_count = GetTickCount();

    SRPACKET *command_ptr = 0;
    SRPACKET_END *sub_command_ptr = 0;

    bool starting = false;
    while(!dlg->stop_recv_packet)
    {
        memset(msg_ptr, 0x00, msg_length);
        if(dlg->local_socket != INVALID_SOCKET) {
            recv_msg_leng = recvfrom(dlg->local_socket, msg_ptr, msg_length, 0, (sockaddr*)&addr_from, (int*)&addr_length);

            if (recv_msg_leng == SOCKET_ERROR) {       

                if (WSAGetLastError() == WSAEWOULDBLOCK) {
                    Sleep(100);
                } else {   
                    dlg->stop_recv_packet = true;
                }
            } else {

                if (recv_msg_leng > 0) {

                    if (!dlg->is_client && !dlg->use_broadcast) {
                        if (addr_from.sin_addr.s_addr != dlg->remote_addr.sin_addr.s_addr) {
                            continue;
                        }
                    }

                    command_ptr = (SRPACKET*)msg_ptr;

                    switch (command_ptr->maincommand) {
                        case 0x01:
                            switch (command_ptr->subcommand) {
                                case SR_PACKET_SUB_COMMAND_START:
                                    TRACE("RECV :START PACKET:\n");
                                    dlg->recved_packet_numbers = 0;
                                    dlg->recved_total_sending_packet_numbers = 0;
                                    dlg->elasped_time = GetTickCount();
                                    starting = true;
                                    break;
                                case SR_PACKET_SUB_COMMAND_SENDING:
                                    if(!starting) {
                                        dlg->stop_recv_packet = true;
                                    }
                                    dlg->recved_packet_numbers++;
                                    if ((GetTickCount() - ticket_count) >= 100 ) {
                                        dlg->PostMessage(WM_INC_RECV_PACKET);
                                        ticket_count = GetTickCount();
                                    }
                                    break;
                                case SR_PACKET_SUB_COMMAND_END:
                                    TRACE("RECV : END PACKET:\n");
                                    if(!starting) {
                                        dlg->stop_recv_packet = true;
                                    }
                                    sub_command_ptr = (SRPACKET_END*)msg_ptr;
                                    dlg->recved_total_sending_packet_numbers = sub_command_ptr->sended_packet_number;
                                    dlg->stop_recv_packet = true;
                                    dlg->elasped_time = GetTickCount() - dlg->elasped_time;
                                    break;
                                default:
                                    break; // END subcommand
                                }
                            break; 
                        default:
                            break;// END command
                    }
                }
            }
        }
    }
    delete msg_ptr;
    dlg->OnBnClickedStop();
    dlg->PostMessage(WM_INC_RECV_PACKET_RESULT);
    AfxEndThread(0, TRUE);
    return 0;
}


void CPacketSRDlg::update_recved_packet_number()
{
    CString cs_packet_number;

    CStatic *sta_total_packet_number_ptr = static_cast<CStatic*>(GetDlgItem(IDC_STA_SENDPACKET_N2));
    cs_packet_number.Format("%lu", recved_total_sending_packet_numbers);
    sta_total_packet_number_ptr->SetWindowText(cs_packet_number);

    CStatic *sta_recv_packet_number_ptr = static_cast<CStatic*>(GetDlgItem(IDC_STA_RECEIVE_PACKET_N2));
    cs_packet_number.Format("%lu", recved_packet_numbers);
    sta_recv_packet_number_ptr->SetWindowText(cs_packet_number);
}

void CPacketSRDlg::update_recved_packet_result()
{
    update_recved_packet_number();

    CString cs_avg_packet_number;
    CString cs_avg_kb_number;
    CStatic *sta_avg_recv_packet_number_ptr = static_cast<CStatic*>(GetDlgItem(IDC_STA_AVG_RECEIVE_PACKET_NUMBER));
    CStatic *sta_avg_recv_kb_number_ptr = static_cast<CStatic*>(GetDlgItem(IDC_STA_AVG_RECEIVE_KB_NUMBER));
    if (elasped_time/1000) {
        LONG64 seconds = (elasped_time - SR_DELAY_STAR_COMMAND)/1000;
        if (seconds && recved_packet_numbers) {
            cs_avg_packet_number.Format("%lu", (recved_packet_numbers/seconds));
            sta_avg_recv_packet_number_ptr->SetWindowText(cs_avg_packet_number);
            
            DWORD packet_length  = sizeof(SRPACKET_END) + stuff_packet_size;
            LONG64 recv_kb_numbers = (recved_packet_numbers/seconds) * packet_length * 8 / 1024;

            cs_avg_kb_number.Format("%lu", recv_kb_numbers);
            sta_avg_recv_kb_number_ptr->SetWindowText(cs_avg_kb_number);
        } else {
            sta_avg_recv_packet_number_ptr->SetWindowText("0");
            sta_avg_recv_kb_number_ptr->SetWindowText("0");
        }
    } else {
        sta_avg_recv_packet_number_ptr->SetWindowText("0");
        sta_avg_recv_kb_number_ptr->SetWindowText("0");
    }

}

