#include "firebasehandler.h"
#include <QNetworkRequest>
#include <functional>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMessageBox>
#include <QJsonObject>
#include <QVariantMap>
#include <QDebug>

//JSON info
//https://thecodeprogram.com/how-to-use-json-data-with-qt-c--

using namespace std;
QString db_url = "https://tutordatabase-a6d03-default-rtdb.firebaseio.com/";
QString db_clients = "/Clients.json";
QString db_log = "Logs.json";


QString REQ_TYPE = "";
QString FIREBASE_API_KEY = "AIzaSyDJHjwmoEt94HP_UTozX5fokVXa5W6o7iI";


FirebaseHandler::FirebaseHandler(QObject *parent)
    : QObject{parent}
{
    this->loadFunc = loadFunc;
    networkManager = new QNetworkAccessManager(this);

}



FirebaseHandler::~FirebaseHandler()
{
    networkManager->deleteLater();
}

void FirebaseHandler::addEntry(QString UID, double length, double amt, QString date, QString time, QString note, function<void(Entry e)> onComplete)
{
    QVariantMap c;
    c[Entry::KEY_DATE] = date + " " + time;
    c[Entry::KEY_LENGTH] =length;
    c[Entry::KEY_AMT] = amt;
    //todo round this or do something to get rid of decimals
    c[Entry::KEY_STATUS] = "UNPAID";
    c[Entry::KEY_NOTE] = note;

    QString url = db_url + loggedIn_UID + "/Clients/" + UID + "/Logs.json?auth=" + idToken;
    requestPost(url,c,[this,c,UID,onComplete](QNetworkReply * reply){
        if(reply->error() == QNetworkReply::NoError)
        {
            QString response = reply->readAll();
            QJsonDocument rep = QJsonDocument::fromJson(response.toUtf8());
            //This gets one JSON Object where each key is the user unique id
            QJsonObject res = rep.object();
            QString LID = res.value("name").toString();
            qDebug() << "Updated log " << LID << "\n";
            //this is where you add the lesson to the local client variable
            Entry e(c);
            e.setLID(LID);
            e.setCID(UID);
            onComplete(e);
        }
        else // handle error
        {
          showErrorBox("The new log was not saved to firebase!");
        }
    });
}

void FirebaseHandler::updateEntryAttribute(QString CID, QString EID, QVariantMap m){

    QString url = db_url + loggedIn_UID + "/Clients/" + CID + "/Logs/" + EID + ".json?auth=" + idToken;
    requestUpdate(url, m, [this](QNetworkReply * reply){
        if(reply->error() == QNetworkReply::NoError)
        {
            QString response = reply->readAll();
            QJsonDocument rep = QJsonDocument::fromJson(response.toUtf8());
            //this is where you add the lesson to the local client variable
            //This gets one JSON Object where each key is the user unique id
            QJsonObject res = rep.object();
            QString LID = res.value("name").toString();
            qDebug() << "Updated log " << LID << "\n";

        }
        else // handle error
        {
          showErrorBox("The last change to this log was not saved!");
        }
    });
}

void FirebaseHandler::updateEntry(Entry * e)
{
    QVariantMap c;
    c[Entry::KEY_DATE] = e->getDate() + " " + e->getTime();
    c[Entry::KEY_LENGTH] = e->getLength();
    c[Entry::KEY_AMT] = e->getAmount();
    //todo round this or do something to get rid of decimals
    c[Entry::KEY_STATUS] = e->getStatus();
    c[Entry::KEY_NOTE] = e->getNote();

    updateEntryAttribute(e->getCID(), e->getLID(),c);

}

void FirebaseHandler::updateClientAttribute(QString CID, QVariantMap data){

    QString url = db_url + loggedIn_UID + "/Clients/" + CID + ".json?auth=" + idToken;
    requestUpdate(url,data,[this](QNetworkReply* reply){
        if(reply->error() == QNetworkReply::NoError)
        {
            QString response = reply->readAll();
            QJsonDocument rep = QJsonDocument::fromJson(response.toUtf8());
            //This gets one JSON Object where each key is the user unique id
            QJsonObject res = rep.object();
            QString CID = res.value("name").toString();
            qDebug() << "performed update!" << CID << "\n";
        }
        else // handle error
        {
            showErrorBox("Could not submit update request");
        }
    });
}

void FirebaseHandler::updateClient(Client * c)
{
    QVariantMap map;
    map[Client::KEY_NAME] = c->getName();
    map[Client::KEY_PAYINFO] = c->getPayinfo();
    map[Client::KEY_PAYTYPE] = c->getPaytype();
    map[Client::KEY_PHONE] = c->getPhone();
    map[Client::KEY_AMT_OWED] = c->getAmtOwed();
    map[Client::KEY_EMAIL] = c->getEmail();
    map[Client::KEY_RATE] = c->getRate();

    updateClientAttribute(c->getUID(),map);
}

void FirebaseHandler::deleteClient(QString UID, function<void(void)> onComplete)
{

    QNetworkRequest deleteC( (QUrl( db_url + loggedIn_UID + "/Clients/" + UID + ".json?auth=" + idToken +"&x-http-method-override=DELETE" )) );
    deleteC.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/json"));
    QNetworkReply * reply = networkManager->post(deleteC, "");

    //get response of request
    connect(reply, &QNetworkReply::finished, [this, reply, onComplete, UID]() {

        if(reply->error() == QNetworkReply::NoError)
        {
            QString response = reply->readAll();

            //delete the local client
            for(auto it = dbClients.begin(); it != dbClients.end(); it++){
                if (it->getUID() == UID){
                    dbClients.erase(it);
                    break;
                }
            }
            loadFunc(dbClients,"");
            onComplete();
        }
        else // handle error
        {          
          showErrorBox("Could not delete client");
        }
    });
}


void FirebaseHandler::deleteEntry(Entry * e, function<void (Entry * e,int)> onComplete, int clickedRow)
{

    QString url = db_url + loggedIn_UID + "/Clients/" + e->getCID() + "/Logs/" + e->getLID() + ".json?auth=" + idToken;
    requestDelete(url, [this,e, clickedRow, onComplete](QNetworkReply * reply){
        if(reply->error() == QNetworkReply::NoError)
        {
            QString response = reply->readAll();
            //delete the local client
            for(auto it = dbClients.begin(); it != dbClients.end(); it++){
                if (it->getUID() == e->getCID()){
                    //iterate through the logs and delete it
                    for(auto it2 = it->getLogs().begin(); it2 != it->getLogs().end(); it2++){
                        if (it2->getLID() == e->getLID()){
                            it->getLogs().erase(it2);
                            break;
                        }
                    }
                    break;
                }
            }
            onComplete(e, clickedRow);
        }
        else // handle error
        {          
          showErrorBox("Could not delete entry");
        }
    });
}

void FirebaseHandler::getClientData(QString selected)
{

    isLoaded = false;
    QString url = db_url + loggedIn_UID + db_clients +"?auth=" + idToken;

    requestGet(url, [this,selected](QNetworkReply * networkReply){
        if(networkReply->error() == QNetworkReply::NoError)
        {
            QString reply = networkReply->readAll();
            QJsonDocument rep = QJsonDocument::fromJson(reply.toUtf8());
            //This gets one JSON Object where each key is the user unique id
            QJsonObject clients = rep.object();
            //get the list of UIDs
            QStringList UIDs = clients.keys();

            //clear the current list
            dbClients.clear();

            //go through the keys and use them to get the clients from the original json obj
           for(int i = 0; i < UIDs.size(); i++){
               QJsonObject curClient = clients.value(UIDs[i]).toObject();
               QJsonObject jsonLogs = curClient.value(Client::KEY_LOGS).toObject();

               //populate list of logs
               vector<Entry> logs;

               QStringList logIDs = jsonLogs.keys();
               for(int j = 0; j < logIDs.size(); j++){
                    QJsonObject curLog = jsonLogs.value(logIDs[j]).toObject();
                    logs.push_back(Entry(curLog,logIDs[j],UIDs[i]));
               }

               //sort the logs
               sort(logs.begin(),logs.end(), Entry::sortByDate_asc);

               Client newClient(curClient);
               newClient.addLogs(logs);
               newClient.setUID(UIDs[i]);

               dbClients.push_back(newClient);
           }
           isLoaded = true;
           loadFunc(dbClients,selected);

        }
        else // handle error
        {
            QString reply = networkReply->readAll();
            QJsonDocument rep = QJsonDocument::fromJson(reply.toUtf8());
            //This gets one JSON Object where each key is the user unique id
            QJsonObject res = rep.object();
            QJsonObject error = res.value("error").toObject();            
            showErrorBox("Could not retrieve the client data!\n" + error.value("message").toString());
        }
    });

}

void FirebaseHandler::addClient(QString name, QString phone, QString email, QString payType, QString payInfo, double amtOwed, double hourly)
{
    //adding a new item
    QVariantMap c;
    c[Client::KEY_NAME] = name;
    c[Client::KEY_PHONE] = phone;
    c[Client::KEY_EMAIL] = email;
    c[Client::KEY_PAYTYPE] = payType;
    c[Client::KEY_PAYINFO] = payInfo;
    c[Client::KEY_AMT_OWED] = amtOwed;
    c[Client::KEY_RATE] = hourly;

    QString url = db_url + loggedIn_UID + db_clients + "?auth=" + idToken;
    requestPost( url, c, [this](QNetworkReply * reply){
        if(reply->error() == QNetworkReply::NoError)
        {
            QString response = reply->readAll();
            QJsonDocument rep = QJsonDocument::fromJson(response.toUtf8());
            //This gets one JSON Object where each key is the user unique id
            QJsonObject res = rep.object();
            QString CID = res.value("name").toString();
            qDebug() << "Added new client: " << CID << "\n";
            getClientData(CID);
        }
        else // handle error
        {
            showErrorBox("Could not add the new client!");
        }
    } );
}
bool FirebaseHandler::hasLoaded(){
    return isLoaded;
}

void FirebaseHandler::showClients(QListWidget *w)
{
    w->clear();
    for(unsigned int i = 0; i < dbClients.size(); i++){
        w->addItem(dbClients[i].getName());
    }
}
//found out how to make lambdas for the get requests so I dont think I need this anymore
void FirebaseHandler::getRequestResult()
{



}

void FirebaseHandler::requestGet(QString url,function<void (QNetworkReply *)> onComplete)
{
    networkReply = networkManager->get( QNetworkRequest( QUrl(url)));
    connect(networkReply, &QNetworkReply::finished, [=]() {

        if(networkReply->error() == QNetworkReply::NoError)
        {
            onComplete(networkReply);
        }

    });
}

void FirebaseHandler::requestDelete(QString url, function<void (QNetworkReply *)> onComplete)
{
    QNetworkRequest deleteReq( (QUrl(url)) );
    deleteReq.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/json"));
    networkReply = networkManager->deleteResource(deleteReq);
    connect(networkReply, &QNetworkReply::finished, [=]() {
        if(networkReply->error() == QNetworkReply::NoError)
        {
            onComplete(networkReply);
        }
    });
}

void FirebaseHandler::showErrorBox(QString msg)
{
    QMessageBox msgBox;
    msgBox.setText(msg);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}

void FirebaseHandler::requestPost(QString url, QVariantMap data, function<void (QNetworkReply *)> onComplete)
{
    QJsonDocument newClient = QJsonDocument::fromVariant(data);
    QNetworkRequest newClientReq( (QUrl(url)) );
    newClientReq.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/json"));
    networkReply = networkManager->post(newClientReq, newClient.toJson());



    connect(networkReply, &QNetworkReply::finished, [=]() {

        if(networkReply->error() == QNetworkReply::NoError)
        {
            onComplete(networkReply);
        }

    });
}

void FirebaseHandler::requestUpdate(QString url, QVariantMap data, function<void (QNetworkReply *)> onComplete)
{
    QJsonDocument update = QJsonDocument::fromVariant(data);
    QNetworkRequest updateReq( (QUrl(url)) );
    updateReq.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/json"));
    networkReply = networkManager->sendCustomRequest(updateReq, "PATCH", update.toJson());
    connect(networkReply, &QNetworkReply::finished, [=]() {
        if(networkReply->error() == QNetworkReply::NoError)
        {
            onComplete(networkReply);
        }
    });
}

void FirebaseHandler::signUpUser(QString email, QString password)
{
    QString url = "https://identitytoolkit.googleapis.com/v1/accounts:signUp?key=" + FIREBASE_API_KEY;
    //whenever the curl description has "Request Body Paylod" we need to make a QVariant Map
    QVariantMap payload;
    payload["email"] = email;
    payload["password"] = password;
    payload["returnSecureToken"] = true;

    QJsonDocument jsonPayload = QJsonDocument::fromVariant(payload);

    QNetworkRequest signUpReq( (QUrl(url)) );
    signUpReq.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/json"));
    QNetworkReply * reply = networkManager->post(signUpReq, jsonPayload.toJson());

    connect(reply, &QNetworkReply::finished, [=]() {

        if(reply->error() == QNetworkReply::NoError)
        {
            QString response = reply->readAll();

        } else {
            qDebug() << "Could not sign up with email " << email;
        }
    });

}


void FirebaseHandler::signInUser(QString email, QString password, load_funct_ptr loadFunc)
{
    this->loadFunc = loadFunc;

    QString url = "https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key=" + FIREBASE_API_KEY;
    //whenever the curl description has "Request Body Paylod" we need to make a QVariant Map
    QVariantMap payload;
    payload["email"] = email;
    payload["password"] = password;
    payload["returnSecureToken"] = true;

    QJsonDocument jsonPayload = QJsonDocument::fromVariant(payload);

    QNetworkRequest signUpReq( (QUrl(url)) );
    signUpReq.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/json"));
    QNetworkReply * reply = networkManager->post(signUpReq, jsonPayload.toJson());

    connect(reply, &QNetworkReply::finished, [=]() {

        if(reply->error() == QNetworkReply::NoError)
        {
            QString response = reply->readAll();
            QJsonDocument rep = QJsonDocument::fromJson(response.toUtf8());
            //This gets one JSON Object where each key is the user unique id
            QJsonObject res = rep.object();
            loggedIn_UID = res.value("localId").toString();
            idToken = res.value("idToken").toString();
            qDebug()<< "Logged into " << loggedIn_UID;
            int stop = 0;

            //make test client
           // addClient("Test1","123","dude@gmail","","",0,60);
           getClientData();

        } else {
            QString response = reply->readAll();
            QJsonDocument rep = QJsonDocument::fromJson(response.toUtf8());
            //This gets one JSON Object where each key is the user unique id
            QJsonObject res = rep.object();
            QJsonObject error = res.value("error").toObject();
            qDebug() << "Could not sign in with email " << email << error.value("message").toString();
        }

    });
    //reply->deleteLater();
}
