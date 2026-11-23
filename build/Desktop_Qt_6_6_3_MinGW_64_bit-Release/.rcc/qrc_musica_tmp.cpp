/****************************************************************************
** Resource object code
**
** Created by: The Resource Compiler for Qt version 6.6.3
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

  // C:/Users/Jose Gabriel Giraldo/OneDrive/Documentos/Universidad/Sexto semestre/Info/proyecto_final/recursos/musica.mp3
  
  
  
static const unsigned char qt_resource_data[5471089] = { 'Q', 'R', 'C', '_', 'D', 'A', 'T', 'A' };

static const unsigned char qt_resource_name[] = {
  // musica.mp3
  0x0,0xa,
  0xf,0x9e,0xcd,0x13,
  0x0,0x6d,
  0x0,0x75,0x0,0x73,0x0,0x69,0x0,0x63,0x0,0x61,0x0,0x2e,0x0,0x6d,0x0,0x70,0x0,0x33,
  
};

static const unsigned char qt_resource_struct[] = {
  // :
  0x0,0x0,0x0,0x0,0x0,0x2,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x1,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  // :/musica.mp3
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,
0x0,0x0,0x1,0xa2,0x6,0x77,0x5c,0x86,

};

#ifdef QT_NAMESPACE
#  define QT_RCC_PREPEND_NAMESPACE(name) ::QT_NAMESPACE::name
#  define QT_RCC_MANGLE_NAMESPACE0(x) x
#  define QT_RCC_MANGLE_NAMESPACE1(a, b) a##_##b
#  define QT_RCC_MANGLE_NAMESPACE2(a, b) QT_RCC_MANGLE_NAMESPACE1(a,b)
#  define QT_RCC_MANGLE_NAMESPACE(name) QT_RCC_MANGLE_NAMESPACE2( \
        QT_RCC_MANGLE_NAMESPACE0(name), QT_RCC_MANGLE_NAMESPACE0(QT_NAMESPACE))
#else
#   define QT_RCC_PREPEND_NAMESPACE(name) name
#   define QT_RCC_MANGLE_NAMESPACE(name) name
#endif

#ifdef QT_NAMESPACE
namespace QT_NAMESPACE {
#endif

bool qRegisterResourceData(int, const unsigned char *, const unsigned char *, const unsigned char *);
bool qUnregisterResourceData(int, const unsigned char *, const unsigned char *, const unsigned char *);

#ifdef QT_NAMESPACE
}
#endif

int QT_RCC_MANGLE_NAMESPACE(qInitResources_musica)();
int QT_RCC_MANGLE_NAMESPACE(qInitResources_musica)()
{
    int version = 3;
    QT_RCC_PREPEND_NAMESPACE(qRegisterResourceData)
        (version, qt_resource_struct, qt_resource_name, qt_resource_data);
    return 1;
}

int QT_RCC_MANGLE_NAMESPACE(qCleanupResources_musica)();
int QT_RCC_MANGLE_NAMESPACE(qCleanupResources_musica)()
{
    int version = 3;
    QT_RCC_PREPEND_NAMESPACE(qUnregisterResourceData)
       (version, qt_resource_struct, qt_resource_name, qt_resource_data);
    return 1;
}

#ifdef __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wexit-time-destructors"
#endif

namespace {
   struct initializer {
       initializer() { QT_RCC_MANGLE_NAMESPACE(qInitResources_musica)(); }
       ~initializer() { QT_RCC_MANGLE_NAMESPACE(qCleanupResources_musica)(); }
   } dummy;
}

#ifdef __clang__
#   pragma clang diagnostic pop
#endif
