#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFormLayout>
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QString>

#include <curl/curl.h>

#include "taskloginwidget.h"
#include "taskmanagementwidget.h"
#include "knossos-global.h"

extern stateInfo *state;

TaskLoginWidget::TaskLoginWidget(QWidget *parent) :
    QDialog(parent), taskManagementWidget(NULL)
{
    setWindowIcon(QIcon(":/images/icons/task.png"));
    setWindowTitle("Task Login");
    setMinimumSize(308, 151);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    QFormLayout *formLayout = new QFormLayout();

    urlField = new QLineEdit();
    urlField->setText(state->taskState->host);
    usernameField = new QLineEdit();
    passwordField = new QLineEdit();
    serverStatus = new QLabel("Please Login");
    passwordField->setEchoMode(QLineEdit::Password);
    loginButton = new QPushButton("Login");

    QLabel *hostLabel = new QLabel("Host:");
    QLabel *usernameLabel = new QLabel("Username:");
    QLabel *passwordLabel = new QLabel("Password");

    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    formLayout->addWidget(serverStatus);
    formLayout->addRow(hostLabel, urlField);
    formLayout->addWidget(line);
    formLayout->addRow(usernameLabel, usernameField);
    formLayout->addRow(passwordLabel, passwordField);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(loginButton);

    setLayout(mainLayout);

    connect(urlField, SIGNAL(editingFinished()), this, SLOT(urlEditingFinished()));
    connect(passwordField, SIGNAL(returnPressed()), this, SLOT(loginButtonClicked()));
    connect(usernameField, SIGNAL(returnPressed()), this, SLOT(loginButtonClicked()));
    connect(urlField, SIGNAL(returnPressed()), this, SLOT(loginButtonClicked()));
    connect(loginButton, SIGNAL(clicked()), this, SLOT(loginButtonClicked()));
}

void TaskLoginWidget::urlEditingFinished() {
    state->taskState->host = urlField->text();
}

void TaskLoginWidget::loginButtonClicked() {
    CURLcode code;
    long httpCode;
    struct httpResponse response;
    FILE *cookie;

    char username[512];
    char password[512];
    char postdata[1024];

    memset(username, '\0', 512);
    memset(password, '\0', 512);
    strcpy(username, usernameField->text().toStdString().c_str());
    strcpy(password, passwordField->text().toStdString().c_str());
    sprintf(postdata, "<login><username>%s</username><password>%s</password></login>", username, password);

    // build url to send to
    const auto url = state->taskState->host + "/knossos/session/";

    // prepare http response object
    response.length = 0;
    response.content = (char *)calloc(1, response.length+1);

    // remove contents of cookie file to fill it with new cookie
    cookie = fopen(state->taskState->cookieFile.toUtf8().constData(), "w");
    if(cookie) {
        fclose(cookie);
    }
    setCursor(Qt::WaitCursor);
    bool result = taskState::httpPOST(url.toUtf8().data(), postdata, &response, &httpCode, state->taskState->cookieFile.toUtf8().data(), &code, 5);
    setCursor(Qt::ArrowCursor);
    if( result == false) {
        serverStatus->setText("<font color='red'>Failed to create cookie. Please check your folder permissions.</font>");
    }
    else if(code == CURLE_OK) {
        if(httpCode == 200) {
            QXmlStreamReader xml(response.content);
            if(xml.hasError()) {
                serverStatus->setText("<font color='red'>Error in transmission. Please try again.</font>");
                return;
            }
            xml.readNextStartElement();
            if(xml.isStartElement() == false) {
                serverStatus->setText("<font color='red'>Error in transmission. Please try again.</font>");
                return;
            }
            // transmission successful
            this->hide();
            if(xml.name() == "task") {
                QString attribute;
                QXmlStreamAttributes attributes = xml.attributes();
                attribute = attributes.value("taskname").toString();
                if(attribute.isNull() == false) {
                    taskManagementWidget->mainTab->setTask(attribute);
                }
                attribute = attributes.value("taskfile").toString();
                if(attribute.isNull() == false) {
                    state->taskState->taskFile = attribute;
                }
                attribute = QByteArray::fromBase64(attributes.value("description").toUtf8());
                if(attribute.isNull() == false) {
                    taskManagementWidget->detailsTab->setDescription(attribute);
                }
                attribute = QByteArray::fromBase64(attributes.value("comment").toUtf8());
                if(attribute.isNull() == false) {
                    taskManagementWidget->detailsTab->setComment(attribute);
                }
            }
            this->hide();
            taskManagementWidget->mainTab->setResponse(QString("Hello %1!").arg(username));
            taskManagementWidget->mainTab->setActiveUser(username);
            taskManagementWidget->show();
            return;
        } else {
            serverStatus->setText(QString("<font color='red'>%1</font>").arg(response.content));
        }
    } else { // !CURLE_OK
        serverStatus->setText("<font color='red'>Request failed. Please check your connection.</font>");
    }
    free(response.content);
}

void TaskLoginWidget::setResponse(QString message) {
    serverStatus->setText(message);
}

void TaskLoginWidget::setTaskManagementWidget(TaskManagementWidget *management) {
    delete taskManagementWidget;
    taskManagementWidget = management;
}

void TaskLoginWidget::closeEvent(QCloseEvent *) {
    this->hide();
}
