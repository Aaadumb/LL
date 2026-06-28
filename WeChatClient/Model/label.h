#ifndef LABEL_H
#define LABEL_H

#include <QObject>
#include <QLabel>

class Label : public QLabel
{
    Q_OBJECT
public:
    explicit Label(QWidget* parent = nullptr);

    void setText(QString text);
private:
    QString AutoFeed(QString s);
};

#endif // LABEL_H
