#define byte unsigned char
#define l_long long double

//������ ���������� ��� ������� �����������������
#define EXPERIMENTAL

#define pFrame byte*

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



#pragma endregion

#define TIMER_RENDER 342

