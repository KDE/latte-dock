#ifndef NOWDOCKPACKAGE_H
#define NOWDOCKPACKAGE_H

#include <QObject>

#include <KPackage/PackageStructure>

namespace Latte {
class DockPackage : public KPackage::PackageStructure {
    Q_OBJECT

public:
    explicit DockPackage(QObject *parent = 0, const QVariantList &args = QVariantList());
    
    ~DockPackage() override;
    void initPackage(KPackage::Package *package) override;
    void pathChanged(KPackage::Package *package) override;
};

}
#endif // NOWDOCKPACKAGE_H
