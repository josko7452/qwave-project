#ifndef TIMESPINBOX_H
#define TIMESPINBOX_H

#include <QDoubleSpinBox>
#include "../../Datamodel/Constants.h"

/**
 * @brief The TimeSpinBox class inherits Qt's QDoubleSpinBox and adds validator and convertor for ps/ns/ms etc.
 */
class TimeSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
private:
    double valueFromText ( const QString & text ) const;
    QString textFromValue ( double value ) const;
    QValidator::State validate ( QString & input, int & pos ) const;

public:
    TimeSpinBox(QWidget * parent = 0);
};

#endif // TIMESPINBOX_H
