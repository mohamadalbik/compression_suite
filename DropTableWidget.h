#ifndef DROPTABLEWIDGET_H
#define DROPTABLEWIDGET_H

#include <QTableWidget>
#include <QPainter>
#include <QDragEnterEvent>
#include <QMimeData>

class DropTableWidget : public QTableWidget
{
    Q_OBJECT

public:
    explicit DropTableWidget(QWidget *parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;

private:
    bool isEmptyTable() const;
    bool m_dragActive = false;


signals:
    void filesDropped(const QString &filePath);
};

#endif // DROPTABLEWIDGET_H
