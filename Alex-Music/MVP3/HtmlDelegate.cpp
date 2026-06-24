
// HtmlDelegate.cpp
#include "HtmlDelegate.h"
#include <QPainter>
#include <QTextDocument>
#include <QApplication>
#include <QAbstractTextDocumentLayout>

HtmlDelegate::HtmlDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {
}

void HtmlDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                         const QModelIndex& index) const {
    QStyleOptionViewItem options = option;
    initStyleOption(&options, index);

    // Проверяем, содержит ли элемент HTML
    QString text = index.data(Qt::DisplayRole).toString();
    bool isHtml = text.contains("<") && text.contains(">");

    if (!isHtml) {
        // Обычный текст - используем стандартный делегат
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    painter->save();

    QTextDocument doc;
    doc.setHtml(text);
    doc.setDefaultFont(options.font);
    doc.setTextWidth(options.rect.width());

    // Отрисовываем фон
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    } else {
        painter->fillRect(option.rect, option.palette.base());
    }

    // Отрисовываем текст
    QRect textRect = options.rect.adjusted(2, 2, -2, -2);
    painter->translate(textRect.topLeft());
    doc.drawContents(painter, QRect(0, 0, textRect.width(), textRect.height()));
    painter->restore();
}

QSize HtmlDelegate::sizeHint(const QStyleOptionViewItem& option,
                             const QModelIndex& index) const {
    QString text = index.data(Qt::DisplayRole).toString();
    bool isHtml = text.contains("<") && text.contains(">");

    if (!isHtml) {
        return QStyledItemDelegate::sizeHint(option, index);
    }

    QTextDocument doc;
    doc.setHtml(text);
    doc.setDefaultFont(option.font);
    doc.setTextWidth(option.rect.width());

    return QSize(doc.idealWidth(), doc.size().height());
}
