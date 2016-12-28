#ifndef NOWDOCKPACKAGE_H
#define NOWDOCKPACKAGE_H

#include <QObject>

#include <KPackage/PackageStructure>

class NowDockPackage : public KPackage::PackageStructure {
    Q_OBJECT

public:
    explicit NowDockPackage(QObject *parent = 0, const QVariantList &args = QVariantList());
    
    ~NowDockPackage() override;
    void initPackage(KPackage::Package *package) override;
    void pathChanged(KPackage::Package *package) override;
};
#endif // NOWDOCKPACKAGE_H
