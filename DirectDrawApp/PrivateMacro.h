#define byte unsigned char
#define l_long long double

//������ ���������� ��� ������� �����������������
#define EXPERIMENTAL

//��������� ��� ������� ��� �� ��������������
#define NYI

//��������. ��������� �������� � ���������� ����� ������������ ����. ��� ����� �������� �������������� � �����������������
//#define FIXEDCORE

//��������. �������� �������������� ����� �������. ����� ��� �������� ����
#define AUTORENDER 0

#define AR_FPSLIMIT 20

//���������. ��������� ������ ������ ����� �������. � ���� ���� ��������� � ���� �� ���������, ��������� ������� ���� �� �������� �� ������ �����, ���� ����� ���������� �������� �� ������� ����.
//�� �������������� �� ���� ;)
#define ENGINEWIDTH 1920
#define ENGINEHEIGHT 1080


#if (ENGINEHEIGHT % 2) != 0
#error "� ������� ���������� ����� ���������� ������"
#endif

#ifdef FIXEDCORE
//���� � ��� ������������ ���������� ����, ��������� ������� ����� ������ �������...
#define RENDERCORES 8
#endif


#pragma region DrawEngineStateNumbers

#define STATE_IDLE 0
#define STATE_DDRAWCREATEFAILED 1
//Undocumentated state's between 1 to 7 - some init error's. Must be catched only in debugger
#define STATE_RENDER_IN_PROGRESS 25
#define STATE_RENDER_FINISHED 42
#define STATE_REQUEST_NEW_FRAME 14
#define STATE_SHUTINGDOWN 55

#pragma endregion

#define TIMER_RENDER 342

