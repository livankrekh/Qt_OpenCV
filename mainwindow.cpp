#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    isPlaying(false)
{
    ui->setupUi(this);

    ui->video->setScene(new QGraphicsScene(this));
    ui->video->scene()->addItem(&pixmap);
}

MainWindow::~MainWindow()
{
    video.release();
    delete ui;
}

void MainWindow::on_openVideo_clicked()
{
    QImage img;
    cv::Mat first;

    QString filename = QFileDialog::getOpenFileName(this, tr("Open Video"), QDir().currentPath(), tr("All files (*)"));

    if (filename == "") {
        return ;
    }

    video.release();
    if (!video.open(filename.toStdString())) {
        QMessageBox::critical(this, "Video Error", "Cannot open video, make sure that this is video file and this is correct file!");
        return ;
    }
    video >> first;

    if (!first.empty()) {
        cv::cvtColor(first, first, CV_BGR2RGB);
        img = getQImage(first);
        pixmap.setPixmap( QPixmap::fromImage(img.rgbSwapped()) );
        ui->video->fitInView(&pixmap, Qt::KeepAspectRatio);
    }
}

void MainWindow::on_stopStart_clicked()
{
    QImage img_frame;
    cv::Mat frame;

    if (isPlaying) {
        isPlaying = false;
        return ;
    }

    isPlaying = true;

    while (video.isOpened()) {

        if (!isPlaying) {
            break ;
        }
        video >> frame;

        if (!frame.empty()) {
            cv::cvtColor(frame, frame, CV_BGR2RGB);
            img_frame = getQImage(frame);

            pixmap.setPixmap( QPixmap::fromImage(img_frame.rgbSwapped()) );
            ui->video->fitInView(&pixmap, Qt::KeepAspectRatio);
        }
        qApp->processEvents();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    isPlaying = false;
    if (video.isOpened()) {
        video.release();
    }

    event->accept();
    QApplication::exit();
}

QImage MainWindow::getQImage(cv::Mat &timage)
{
    static QVector<QRgb> colorTable;

    if (colorTable.isEmpty()) {
        for (int i = 0; i < 256; i++) {
            colorTable.push_back(qRgb(i, i, i));
        }
    }

    if (timage.type() == CV_8UC3) {
        QImage temp = QImage((const unsigned char*)(timage.data), timage.cols, timage.rows, timage.step, QImage::Format_RGB888);
        return temp.rgbSwapped();
    }
    else if (timage.type() == CV_8UC1) {
        QImage temp = QImage((const unsigned char*)(timage.data), timage.cols, timage.rows, timage.step, QImage::Format_Indexed8);
        temp.setColorTable(colorTable);
        return temp;
    }
}
