#include "mainwindow.h"
#include "ui_mainwindow.h"


cv::Mat image_1, image_2;
cv::Size img_size(500,500);
int x = -1, y = -1, x_prime = -1, y_prime = -1;
int original_points = 0 , desired_points = 0;
int grid = 15;

int points_x[4], points_y[4], points_x_prime[4], points_y_prime[4];
//vars used to solve the system
cv::Mat LHS = cv::Mat(8,8, CV_32F);
cv::Mat RHS = cv::Mat(8,1, CV_32F);
cv::Mat params = cv::Mat(8,1, CV_32F);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void reset(){
    x= -1; y = -1; x_prime=-1; y_prime=-1;
    original_points = 0; desired_points= 0;
}

void putOriginalPoint(int event, int x, int y, int flags, void* userdata)
{
    QTableWidget * table = (QTableWidget *) userdata;

    if  ( event == cv::EVENT_LBUTTONDOWN )
    {
         std::cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << std::endl;
         if(original_points == table->rowCount()){
             table->insertRow(original_points);
         }
         ::x = x;
         ::y = y;
         //put on the table
         QTableWidgetItem * item = new QTableWidgetItem(QString::number(::x));
         table->setItem(original_points,0, item);
         item = new QTableWidgetItem(QString::number(::y));
         table->setItem(original_points,1, item);
         if(original_points < 4){
            points_x[original_points] = ::x;
            points_y[original_points] = ::y;
            //draw the point in the image
            cv::circle(image_2,cv::Point(x,y),3, cv::Scalar(0,255,0),-1);
            cv::imshow("View", image_2);
         }
         original_points++;
    }
    else if (event == cv::EVENT_MBUTTONDOWN )
    {
        std::cout << "Right button of the mouse is clicked - position (" << x << ", " << y << ")" << std::endl;
        if(desired_points == table->rowCount()){
            table->insertRow(desired_points);
        }
        ::x_prime = x;
        ::y_prime = y;
        //put on the table
        QTableWidgetItem * item = new QTableWidgetItem(QString::number(x_prime));
        table->setItem(desired_points,2, item);
        item = new QTableWidgetItem(QString::number(y_prime));
        table->setItem(desired_points,3, item);
        if(desired_points < 4){
           points_x_prime[desired_points] = ::x_prime;
           points_y_prime[desired_points] = ::y_prime;
           //draw the point in the image
           cv::circle(image_2,cv::Point(x,y),3, cv::Scalar(0,0,255), -1);
           cv::imshow("View", image_2);
        }
        desired_points++;

    }
}




void MainWindow::on_loadButton_clicked()
{
    //load an image to compare
    std::cout<< "Loading an image to compare"<< std::endl;
    std::string url = ui->inputText->text().toStdString();
    image_1 = cv::imread(url.c_str());
    cv::resize(image_1, image_1, img_size, CV_INTER_CUBIC);

    //Draw a grid
    image_2 = image_1.clone();
    int width  = img_size.width;
    int height = img_size.height;
    int i=0;
    for(i=0;i<height;i+=height/grid)
      cv::line(image_2, cv::Point(0,i), cv::Point(width,i), cv::Scalar(255,0,0));

    for(i=0;i<width;i+=width/grid)
      cv::line(image_2, cv::Point(i,0), cv::Point(i,height), cv::Scalar(255,0,0));

    cv::namedWindow("View", cv::WINDOW_AUTOSIZE);
    cv::imshow("View", image_2);
    //set the callback function for any mouse event
    cv::setMouseCallback("View", putOriginalPoint, ui->table);
}


void MainWindow::on_applyButton_clicked()
{
    int c = 0;
    for(c = 0; c < 4; c++){
        std::cout<<" x : " << points_x[c] << " \t";
    }
    std::cout << std::endl;
    for(c = 0; c < 4; c++){
        std::cout<<" y : " << points_y[c] << " \t";
    }
    std::cout << std::endl;
    for(c = 0; c < 4; c++){
        std::cout<<" x_prime : " << points_x_prime[c] << " \t";
    }
    std::cout << std::endl;
    for(c = 0; c < 4; c++){
        std::cout<<" y_prime : " << points_y_prime[c] << " \t";
    }
    std::cout << std::endl;
    //convert to mat
    LHS = cv::Mat::zeros(8,8, CV_32F);
    RHS = cv::Mat::zeros(8,1, CV_32F);
    for(c = 0; c < 4; c++){
        LHS.at<float>(2*c,0) = points_x[c];
        LHS.at<float>(2*c,1) = points_y[c];
        LHS.at<float>(2*c,2) = 1;
        LHS.at<float>(2*c,6) = -points_x[c]*points_x_prime[c];
        LHS.at<float>(2*c,7) = -points_y[c]*points_x_prime[c];
        RHS.at<float>(2*c,0) = points_x_prime[c];

        LHS.at<float>(2*c+1,3) = points_x[c];
        LHS.at<float>(2*c+1,4) = points_y[c];
        LHS.at<float>(2*c+1,5) = 1;
        LHS.at<float>(2*c+1,6) = -points_x[c]*points_y_prime[c];
        LHS.at<float>(2*c+1,7) = -points_y[c]*points_y_prime[c];
        RHS.at<float>(2*c+1,0) = points_y_prime[c];
    }
    //print mats
    std::cout << " LHS = " << LHS << std::endl;
    std::cout << " RHS = " << RHS << std::endl;

    //Solve the system
    cv::solve(LHS,RHS,params);

    //Construct Perpective Matrix
    cv::Mat perspective = cv::Mat(3,3, CV_32F);
    perspective.at<float>(0,0) = params.at<float>(0,0);
    perspective.at<float>(0,1) = params.at<float>(0,1);
    perspective.at<float>(0,2) = params.at<float>(0,2);
    perspective.at<float>(1,0) = params.at<float>(0,3);
    perspective.at<float>(1,1) = params.at<float>(0,4);
    perspective.at<float>(1,2) = params.at<float>(0,5);
    perspective.at<float>(2,0) = params.at<float>(0,6);
    perspective.at<float>(2,1) = params.at<float>(0,7);
    perspective.at<float>(2,2) = 1;

    //get perspective transform
    cv::Point2f points[4];
    points[0] = cv::Point2f(points_x[0], points_y[0]);
    points[1] = cv::Point2f(points_x[1], points_y[1]);
    points[2] = cv::Point2f(points_x[2], points_y[2]);
    points[3] = cv::Point2f(points_x[3], points_y[3]);
    points[4] = cv::Point2f(points_x[4], points_y[4]);

    cv::Point2f points_d[4];
    points_d[0] = cv::Point2f(points_x_prime[0], points_y_prime[0]);
    points_d[1] = cv::Point2f(points_x_prime[1], points_y_prime[1]);
    points_d[2] = cv::Point2f(points_x_prime[2], points_y_prime[2]);
    points_d[3] = cv::Point2f(points_x_prime[3], points_y_prime[3]);
    points_d[4] = cv::Point2f(points_x_prime[4], points_y_prime[4]);

    //Apply transformation
    cv::Mat result = image_1.clone();

    std::cout << " Perspective = " << perspective << std::endl;

    /*OpenCV has a function to automatically get the Perpective matrix*/
    //perspective = cv::getPerspectiveTransform(points, points_d);

    //std::cout << " Perspective = " << perspective << std::endl;

    cv::warpPerspective(image_1,result,perspective, img_size, CV_INTER_CUBIC);
    //cv::resize(result, result, img_size, CV_INTER_CUBIC);
    cv::namedWindow("Result", cv::WINDOW_AUTOSIZE);
    cv::imshow("Result", result);
    cv::waitKey(0);
    cv::destroyWindow("Result");
}

void MainWindow::on_clearButton_clicked()
{
    reset();
    while(ui->table->rowCount() > 0) ui->table->removeRow(0);
    //Draw a grid
    image_2 = image_1.clone();
    int width  = img_size.width;
    int height = img_size.height;
    int i=0;
    for(i=0;i<height;i+=height/grid)
      cv::line(image_2, cv::Point(0,i), cv::Point(width,i), cv::Scalar(255,0,0));

    for(i=0;i<width;i+=width/grid)
      cv::line(image_2, cv::Point(i,0), cv::Point(i,height), cv::Scalar(255,0,0));

    cv::namedWindow("View", cv::WINDOW_AUTOSIZE);
    cv::imshow("View", image_2);
}
