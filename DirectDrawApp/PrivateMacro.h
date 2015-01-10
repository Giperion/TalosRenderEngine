#define byte unsigned char
#define l_long long double

//Просто показатель что функция экспериментальная
#define EXPERIMENTAL

#define pFrame byte*

//Указывает что функция еще не заимплементена
#define NYI

//Параметр. Позволяет включать в приложений режим фиксированых ядер. Его можно спокойно комментировать и расскоментировать
//#define FIXEDCORE

//Параметр. Включает автоматический вызов рендера. Лимит фпс задается ниже
#define AUTORENDER 0

#define AR_FPSLIMIT 20

//Параметры. Позволяют менять размер кадра рендера. И хотя окно создается с теми же размерами, изменения размера окна не повлияет на размер кадра, кадр будет программно увеличен до размера окна.
//Но комментировать их низя ;)
#define ENGINEWIDTH 1920
#define ENGINEHEIGHT 1080


#if (ENGINEHEIGHT % 2) != 0
#error "В текущей реализации нужна нормальная высота"
#endif

#ifdef FIXEDCORE
//Если у нас фиксированое количество ядер, указываем сколько нужно рендер потоков...
#define RENDERCORES 8
#endif


#pragma region DrawEngineStateNumbers



#pragma endregion

#define TIMER_RENDER 342

