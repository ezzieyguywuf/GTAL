#ifndef ICylinder_H
#define ICylinder_H

#include <ISolid.h>
#include <IFace.h>
#include <BKALTypes.h>

namespace BKAL{
    class ICylinder : public virtual ISolid
    {
        public:
            virtual inline ~ICylinder() = default;
            virtual const IFace& lateral() const = 0;
            virtual const IFace& top() const = 0;
            virtual const IFace& bottom() const = 0;
    };
}

#endif /* ICylinder_H */
