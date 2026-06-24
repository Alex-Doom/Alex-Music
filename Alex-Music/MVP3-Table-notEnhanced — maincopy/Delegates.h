#pragma once
#include <QStyledItemDelegate>
#include <QMouseEvent>

class HtmlDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit HtmlDelegate(QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

class RatingDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit RatingDelegate(QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model,
                     const QStyleOptionViewItem& option, const QModelIndex& index) override;

signals:
    void ratingChanged(int row, int rating);
};
