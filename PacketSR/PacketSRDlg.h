// PacketSRDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include <winsock2.h>
#include <IPHlpApi.h>
#include <vector>
#include <set>

#define WM_INC_SEND_PACKET (WM_USER + 100)
#define WM_INC_RECV_PACKET (WM_USER + 101)
#define WM_INC_RECV_PACKET_RESULT (WM_USER + 102)

#pragma pack(1)

#define SR_PACKET_MAIN_COMMAND        0x01
#define SR_PACKET_SUB_COMMAND_START   0x01
#define SR_PACKET_SUB_COMMAND_SENDING 0x02
#define SR_PACKET_SUB_COMMAND_END     0x03
#define SR_DELAY_STAR_COMMAND         3000
struct SRPACKET {
    unsigned char maincommand;
    unsigned char subcommand;
};

struct SRPACKET_END {
    SRPACKET command;
    LONG64 sended_packet_number; 
};

#pragma pack()

// CPacketSRDlg dialog
class CPacketSRDlg : public CDialog
{
// Construction
public:
	CPacketSRDlg(CWnd* pParent = NULL);	// standard constructor
    ~CPacketSRDlg();
// Dialog Data
	enum { IDD = IDD_PACKETSR_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    virtual void OnCancel();
    afx_msg void OnClose();
    afx_msg void OnBnClickedStart();
    afx_msg void OnBnClickedStop();
    afx_msg void OnBnClickedRadioBroadcast();
    afx_msg void OnBnClickedRadioUnicast();
    afx_msg void OnCbnSelchangeCombo1();
    afx_msg void OnBnClickedRadioSendPacket();
    afx_msg void OnBnClickedRadioRecvPacket();
	DECLARE_MESSAGE_MAP()

    CStatic sta_sended_packet_number_;
    CIPAddressCtrl ipc_remote_address;
    //CIPAddressCtrl ipc_from_address;
    CComboBox ccbox_nic_list;
    CStatic avg_packet_one_seconds;
    CButton btn_start;
    CButton btn_stop;

    WSADATA wsaData;
    SOCKET  local_socket;
    BOOL use_broadcast;
    BOOL is_client;
    bool stop_send_packet;
    bool stop_recv_packet;
    SOCKADDR_IN remote_addr;
    LONG64 sended_packet_numbers;
    LONG64 recved_packet_numbers;
    LONG64 recved_total_sending_packet_numbers;
    CString cs_nic_ip;
    unsigned int port_number;
    DWORD elasped_time;
    int stuff_packet_size;

    static UINT create_send_packet(void *void_ptr);
    static UINT create_recv_packet(void *void_ptr);
    void update_sended_packet_number();
    void update_recved_packet_number();
    void update_recved_packet_result();
    void detect_ip_list();
};
