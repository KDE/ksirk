/*
 * simplesasl.cpp - Simple SASL implementation
 * Copyright (C) 2003  Justin Karneges
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * either version 2
   of the License, or (at your option) any later version.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "simplesasl.h"

#include <QHostAddress>
#include <QStringList>
#include <QList>
#include <qca.h>
#include <QByteArray>
#include <stdlib.h>
#include <QtCrypto>
#include <QDebug>

#include "xmpp/sasl/plainmessage.h"
#include "xmpp/sasl/digestmd5response.h"
#include "xmpp/base/randrandomnumbergenerator.h"

namespace XMPP {
class SimpleSASLContext : public QCA::SASLContext
{
public:
		class ParamsMutable
		{
		public:
			/**
			   User is held
			*/
			bool user;

			/**
			   Authorization ID is held
			*/
			bool authzid;

			/**
			   Password is held
			*/
			bool pass;

			/**
			   Realm is held
			*/
			bool realm;
		};
	// core props
	QString service, host;

	// state
	int step;
	bool capable;
	bool allow_plain;
	QByteArray out_buf, in_buf;
	QString mechanism_;
	QString out_mech;

	ParamsMutable need;
	ParamsMutable have;
	QString user, authz, realm;
	QCA::SecureArray pass;
	Result result_;
	QCA::SASL::AuthCondition authCondition_;
	QByteArray result_to_net_, result_to_app_;
	int encoded_;

	SimpleSASLContext(QCA::Provider* p) : QCA::SASLContext(p)
	{
		reset();
	}

	~SimpleSASLContext() override
	{
		reset();
	}

	void reset() override
	{
		resetState();

		capable = true;
		allow_plain = false;
		need.user = false;
		need.authzid = false;
		need.pass = false;
		need.realm = false;
		have.user = false;
		have.authzid = false;
		have.pass = false;
		have.realm = false;
		user = QString();
		authz = QString();
		pass = QCA::SecureArray();
		realm = QString();
	}

	void resetState()
	{
		out_mech = QString();
		out_buf.resize(0);
		authCondition_ = QCA::SASL::AuthFail;
	}

	void setConstraints(QCA::SASL::AuthFlags flags, int ssfMin, int) override {
		if(flags & (QCA::SASL::RequireForwardSecrecy | QCA::SASL::RequirePassCredentials | QCA::SASL::RequireMutualAuth) || ssfMin > 0)
			capable = false;
		else
			capable = true;
		allow_plain = flags & QCA::SASL::AllowPlain;
	}
	
	void setup(const QString& _service, const QString& _host, const QCA::SASLContext::HostPort*, const QCA::SASLContext::HostPort*, const QString&, int) override {
		service = _service;
		host = _host;
	}
	
	void startClient(const QStringList &mechlist, bool allowClientSendFirst) override {
		Q_UNUSED(allowClientSendFirst);

		mechanism_ = QString();
		foreach(QString mech, mechlist) {
			if (mech == "DIGEST-MD5") {
				mechanism_ = "DIGEST-MD5";
				break;
			}
			if (mech == "PLAIN" && allow_plain) 
				mechanism_ = "PLAIN";
		}

		if(!capable || mechanism_.isEmpty()) {
			result_ = Error;
			authCondition_ = QCA::SASL::NoMechanism;
			if (!capable)
				qWarning("simplesasl.cpp: Not enough capabilities");
			if (mechanism_.isEmpty()) 
				qWarning("simplesasl.cpp: No mechanism available");
			QMetaObject::invokeMethod(this, "resultsReady", Qt::QueuedConnection);
			return;
		}

		resetState();
		result_ = Continue;
		step = 0;
		tryAgain();
	}

	void nextStep(const QByteArray &from_net) override {
		in_buf = from_net;
		tryAgain();
	}

	void tryAgain() override {
		// All exits of the method must emit the ready signal
		// so all exits go through a goto ready; 
		if(step == 0) {
			out_mech = mechanism_;
			
			// PLAIN 
			if (out_mech == "PLAIN") {
				// First, check if we have everything
				if(need.user || need.pass) {
					qWarning("simplesasl.cpp: Did not receive necessary auth parameters");
					result_ = Error;
					goto ready;
				}
				if(!have.user)
					need.user = true;
				if(!have.pass)
					need.pass = true;
				if(need.user || need.pass) {
					result_ = Params;
					goto ready;
				}
				out_buf = PLAINMessage(authz, user, pass.toByteArray()).getValue();
			}
			++step;
			if (out_mech == "PLAIN")
				result_ = Success;
			else
				result_ = Continue;
		}
		else if(step == 1) {
			Q_ASSERT(out_mech != "PLAIN");

			// if we still need params, then the app has failed us!
			if(need.user || need.authzid || need.pass || need.realm) {
				qWarning("simplesasl.cpp: Did not receive necessary auth parameters");
				result_ = Error;
				goto ready;
			}

			// see if some params are needed
			if(!have.user)
				need.user = true;
			//if(!have.authzid)
			//	need.authzid = true;
			if(!have.pass)
				need.pass = true;
			if(need.user || need.authzid || need.pass) {
				result_ = Params;
				goto ready;
			}

			DIGESTMD5Response response(in_buf, service, host, realm, user, authz, pass.toByteArray(), RandRandomNumberGenerator());
			if (!response.isValid()) {
				authCondition_ = QCA::SASL::BadProtocol;
				result_ = Error;
				goto ready;
			}
			out_buf = response.getValue();
			++step;
			result_ = Continue;
		}
		/*else if (step == 2) {
			//Commenting this out is Justin's fix for updated QCA.
			out_buf.resize(0);
			result_ = Continue;
			++step;
		}*/
		else {
			out_buf.resize(0);
			result_ = Success;
		}
ready:
		QMetaObject::invokeMethod(this, "resultsReady", Qt::QueuedConnection);
	}

	void update(const QByteArray &from_net, const QByteArray &from_app) override {
		result_to_app_ = from_net;
		result_to_net_ = from_app;
		encoded_ = from_app.size();
		result_ = Success;
		QMetaObject::invokeMethod(this, "resultsReady", Qt::QueuedConnection);
	}

	bool waitForResultsReady(int msecs) override {

		// TODO: for now, all operations block anyway
		Q_UNUSED(msecs);
		return true;
	}

	Result result() const override {
		return result_;
	}

	QStringList mechlist() const override {
		return QStringList();
	}
	
	QString mech() const override {
		return out_mech;
	}
	
	bool haveClientInit() const override {
		return out_mech == "PLAIN";
	}
	
	QByteArray stepData() const override {
		return out_buf;
	}
	
	QByteArray to_net() override {
		return result_to_net_;
	}
	
	int encoded() const override {
		return encoded_;
	}
	
	QByteArray to_app() override {
		return result_to_app_;
	}

	int ssf() const override {
		return 0;
	}

	QCA::SASL::AuthCondition authCondition() const override {
		return authCondition_;
	}

	QCA::SASL::Params clientParams() const override {
		return QCA::SASL::Params(need.user, need.authzid, need.pass, need.realm);
	}
	
	void setClientParams(const QString *_user, const QString *_authzid, const QCA::SecureArray *_pass, const QString *_realm) override {
		if(_user) {
			user = *_user;
			need.user = false;
			have.user = true;
		}
		if(_authzid) {
			authz = *_authzid;
			need.authzid = false;
			have.authzid = true;
		}
		if(_pass) {
			pass = *_pass;
			need.pass = false;
			have.pass = true;
		}
		if(_realm) {
			realm = *_realm;
			need.realm = false;
			have.realm = true;
		}
	}

	QStringList realmlist() const override
	{
		// TODO
		return QStringList();
	}

	QString username() const override {
		return QString();
	}

	QString authzid() const override {
		return QString();
	}

	QCA::Provider::Context* clone() const override {
		SimpleSASLContext* s = new SimpleSASLContext(provider());
		// TODO: Copy all the members
		return s;
	}
	
	void startServer(const QString &, bool) override {
		result_ =  QCA::SASLContext::Error;
		QMetaObject::invokeMethod(this, "resultsReady", Qt::QueuedConnection);
	}
	void serverFirstStep(const QString &, const QByteArray *) override {
		result_ =  QCA::SASLContext::Error;
		QMetaObject::invokeMethod(this, "resultsReady", Qt::QueuedConnection);
	}

};

class QCASimpleSASL : public QCA::Provider
{
public:
	QCASimpleSASL() {}
	~QCASimpleSASL() override {}

	void init() override
	{
	}

	QString name() const override {
		return "simplesasl";
	}

	QStringList features() const override {
		return QStringList("sasl");
	}

	QCA::Provider::Context* createContext(const QString& cap) override
	{
		if(cap == "sasl")
			return new SimpleSASLContext(this);
		return 0;
	}
	int qcaVersion() const override
	{
		return QCA_VERSION;
	}
};

QCA::Provider *createProviderSimpleSASL()
{
	return (new QCASimpleSASL);
}

}
