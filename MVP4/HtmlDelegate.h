// HtmlDelegate.h
#pragma once
#include <QStyledItemDelegate>

class HtmlDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit HtmlDelegate(QObject* parent = nullptr);

protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override;
};
