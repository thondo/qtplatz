// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef ACQUIREPLUGIN_H
#define ACQUIREPLUGIN_H

#include <extensionsystem/iplugin.h>
#include <boost/smart_ptr.hpp>
# pragma warning(disable:4996)
#  pragma warning(disable:4805)
#   include <adinterface/controlserverC.h>
#  pragma warning(default:4805)
# include <adinterface/signalobserverC.h>
# pragma warning(default:4996)
#include <vector>

class QToolButton;
class QAction;

namespace adplugin {
   class QReceiver_i;
   class QObserverEvents_i;
}

namespace Acquire {
	namespace internal {

    class AcquireUIManager;
	class AcquireImpl;

    //------------
    class AcquirePlugin : public ExtensionSystem::IPlugin {
      Q_OBJECT
    public:
      ~AcquirePlugin();
      AcquirePlugin();

      // implement IPlugin
      virtual bool initialize(const QStringList &arguments, QString *error_message);
      virtual void extensionsInitialized();
      virtual void shutdown();

    private slots:
		void actionConnect();
		void actionDisconnect();
		void actionRunStop();

        void handle_message( unsigned long msg, unsigned long value );
        void handle_log( QByteArray );
        void handle_shutdown();
        void handle_debug_print( unsigned long priority, unsigned long category, QString text );

		void handle_update_data( unsigned long objid, long pos );
    signals:

    private:
      AcquireUIManager * manager_;
	  AcquireImpl * pImpl_;

      QAction * actionConnect_;
      QAction * actionRunStop_;
      QAction * action3_;
      QAction * action4_;
      QAction * action5_;

      void action1();
      void action2();
      void action3();
      void action4();
      void action5();

      void initialize_actions();

      ControlServer::Session_var session_;
	  SignalObserver::Observer_var observer_;
      // for quick debug, to be removed
	  SignalObserver::Observer_var tofCache_;
	  // <--
      boost::scoped_ptr< adplugin::QReceiver_i > receiver_i_;
	  boost::scoped_ptr< adplugin::QObserverEvents_i > observer_i_;
	  std::vector< boost::shared_ptr< adplugin::QObserverEvents_i > > sinkVec_;

    public:
      static QToolButton * toolButton( QAction * action );
    };
  }
}

#endif // ACQUIREPLUGIN_H
