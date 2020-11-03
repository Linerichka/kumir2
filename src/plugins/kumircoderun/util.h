#ifndef UTIL_H
#define UTIL_H

#define DO_NOT_DECLARE_STATIC
#include <kumir2-libs/vm/variant.hpp>
//#include <kumir2/actorinterface.h>
#include <QVariant>
#include <QThread>


namespace Shared {
class ActorInterface;
}

namespace KumirCodeRun
{

namespace Util
{

using namespace VM;
using namespace Kumir;
using namespace Shared;

QVariant VariableToQVariant(const Variable &var);
AnyValue QVariantToValue(const QVariant &var, int dim);

ActorInterface *findActor(const std::string &moduleAsciiName, bool allowLoad = true);
ActorInterface *findActor(const QByteArray &moduleAsciiName, bool allowLoad = true);

class SleepFunctions: private QThread
{
public:
	inline static void msleep(quint32 msec)
	{
		QThread::msleep(msec);
	}
	inline static void usleep(quint32 usec)
	{
		QThread::usleep(usec);
	}
private:
	inline void run() {}
};

}
} // namespaces

#endif // UTIL_H
