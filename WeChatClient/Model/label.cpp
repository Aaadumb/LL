#include "label.h"
#include <QtMath>
#include <QDebug>
Label::Label(QWidget* parent) : QLabel(parent)
{
    QLabel::setWordWrap(true);
    QLabel::setAlignment(Qt::AlignTop | Qt::AlignLeft);
    QLabel::setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

void Label::setText(QString text)
{
	QLabel::setText(AutoFeed(text));
    QLabel::adjustSize();
}
QString Label::AutoFeed(QString text)
{
	if (text.isEmpty())return text;
	// 自己写一个双指针逻辑 用于换行 每18个字符换一行 如果中间遇到换行符 重新计算
	QString strText = "";
	int cnt = 0, slow = 0;
	for (QChar& ch : text)
	{
		if (ch != '\n')
		{
			strText += ch;
			cnt++;
			if (cnt == 18)
			{
				strText += '\n';
				cnt = 0;
			}
		}
		else
		{
			strText += ch;
			cnt = 0;
		}
	}
	return strText;
}
