/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/
#include <adportable/debug.hpp>

// back-end
#include <boost/msm/back/state_machine.hpp>

// front-end
#include <boost/msm/front/state_machine_def.hpp>

// functors
#include <boost/msm/front/functor_row.hpp>
#include <boost/msm/front/euml/common.hpp>

// And_ operator
#include <boost/msm/front/euml/operator.hpp>

// func_state, func_state_machine
#include <boost/msm/front/euml/state_grammar.hpp>

#include <iostream>
#include <functional>
#include <memory>

namespace adacquire {

    namespace fsm {

        namespace msm = boost::msm;
        namespace mpl = boost::mpl;
        using boost::msm::front::Row;
        using boost::msm::front::none;

        enum idState { idStopped, idPreparingForRun, idWaitForContactClosure, idRunning, idDormant };
        static const char * stateNames[] = { "Stopped", "PreparingForRun", "WaitForContactClosure", "Running", "Dormant" };

        // events
        struct Stop        {}; // --> Stop
        struct Start       {}; // --> PreparingForRun
        struct Ready       {}; // --> WaitForContactClosure
        struct Inject      {}; // --> Running
        struct Complete    {}; // --> Dormant
        struct error_clear {};

        struct error_detected {
            error_detected(const std::string& name)  : name_(name)
                {}
            std::string name_;
        };

        struct handler;

        struct controller_ : public msm::front::state_machine_def< controller_ > {

            controller_( handler * p ) : handler_( p ) {}

            struct Error_tag {};
            typedef msm::front::euml::func_state< Error_tag > Error;

            //-----------------------------
            struct Stopped_Entry { 
                template <class Event, class FSM, class STATE> void operator()( Event const& evt, FSM& fsm, STATE& ) {
                    fsm.handler_->fsm_state( true, idStopped, fsm.current_state()[0] );
                }
            };
            struct Stopped_Exit { 
                template <class Event, class FSM, class STATE> void operator()( Event const& evt, FSM& fsm, STATE& ) {
                    fsm.handler_->fsm_state( false, idStopped, fsm.current_state()[0] );
                }
            };
            struct Stopped_tag {};
            typedef msm::front::euml::func_state< Stopped_tag, Stopped_Entry, Stopped_Exit> Stopped;

            //-----------------------------
            struct PreparingForRun_Entry { 
                template <class Event, class FSM, class STATE> void operator()( Event const& evt, FSM& fsm, STATE& ) {
                    fsm.handler_->fsm_state( true, idPreparingForRun, fsm.current_state()[0] );
                }
            };
            struct PreparingForRun_Exit { 
                template <class Event, class FSM, class STATE> void operator()( Event const& evt, FSM& fsm, STATE& ) {
                    fsm.handler_->fsm_state( false, idPreparingForRun, fsm.current_state()[0] );
                }
            };
            struct PreparingForRun_tag {};
            typedef msm::front::euml::func_state< PreparingForRun_tag, PreparingForRun_Entry, PreparingForRun_Exit> PreparingForRun;

            //-----------------------------
            struct WaitForContactClosure_Entry { 
                template <class Event, class FSM, class STATE> void operator()( Event const& evt, FSM& fsm, STATE& ) {
                    fsm.handler_->fsm_state( true, idWaitForContactClosure, fsm.current_state()[0] );
                }
            };
            struct WaitForContactClosure_Exit { 
                template <class Event, class FSM, class STATE> void operator()( Event const& evt, FSM& fsm, STATE& ) {
                    fsm.handler_->fsm_state( false, idWaitForContactClosure, fsm.current_state()[0] );
                }
            };
            struct WaitForContactClosure_tag {};
            typedef msm::front::euml::func_state< WaitForContactClosure_tag
                                                  , WaitForContactClosure_Entry, WaitForContactClosure_Exit> WaitForContactClosure;
            

            //-----------------------------
            struct Running_Entry {
                template <class Event, class FSM, class STATE> void operator()( Event const& evt, FSM& fsm, STATE& ) {
                    fsm.handler_->fsm_state( true, idRunning );
                }
            };
            struct Running_Exit {
                template <class Event, class FSM, class STATE> void operator()( Event const& evt, FSM& fsm, STATE& ) {
                    fsm.handler_->fsm_state( false, idRunning );
                }
            };            
            struct Running_tag {};
            typedef msm::front::euml::func_state< Running_tag, Running_Entry, Running_Exit > Running;

            //-----------------------------
            struct Dormant_Entry { 
                template <class Event, class FSM, class STATE> void operator()( Event const& evt, FSM& fsm, STATE& ) {
                    fsm.handler_->fsm_state( true, idDormant );
                }
            };
            struct Dormant_Exit { 
                template <class Event, class FSM, class STATE> void operator()( Event const& evt, FSM& fsm, STATE& ) {
                    fsm.handler_->fsm_state( false, idDormant );
                }
            };
            struct Dormant_tag {};
            typedef msm::front::euml::func_state< Dormant_tag, Dormant_Entry, Dormant_Exit> Dormant;
                
            //-----------------------------
            //-----------------------------
                
            typedef Stopped initial_state;

            //-----------------------------
            struct actStop {
                template <class EVT, class FSM, class SourceState, class TargetState >
                void operator()( EVT const& evt, FSM& fsm, SourceState&, TargetState& ) {
                    fsm.handler_->fsm_action_stop();
                }
            };

            struct actStart {
                template <class EVT, class FSM, class SourceState, class TargetState >
                void operator()( EVT const& evt, FSM& fsm, SourceState&, TargetState& ) {
                    fsm.handler_->fsm_action_start();
                }
            };

            struct actReady {
                template <class EVT, class FSM, class SourceState, class TargetState >
                void operator()( EVT const& evt, FSM& fsm, SourceState&, TargetState& ) {
                    fsm.handler_->fsm_action_ready();
                }
            };
                
            struct actInject {
                template <class EVT, class FSM, class SourceState, class TargetState >
                void operator()( EVT const& evt, FSM& fsm, SourceState&, TargetState& ) {
                    fsm.handler_->fsm_action_inject();
                }
            };

            struct actComplete {
                template <class EVT, class FSM, class SourceState, class TargetState >
                void operator()( EVT const& evt, FSM& fsm, SourceState&, TargetState& ) {
                    fsm.handler_->fsm_action_complete();
                }
            };

            // guard conditions
            struct DummyGuard {
                template <class EVT,class FSM,class SourceState,class TargetState>
                bool operator()(EVT const& evt,FSM& fsm,SourceState& src,TargetState& tgt) {
                    return true;
                }
            };
            struct good_diag_result {
                template <class EVT,class FSM,class SourceState,class TargetState>
                bool operator()(EVT const& evt ,FSM&,SourceState& ,TargetState& ) {
                    return true;
                }
            };
            struct always_true {
                template <class EVT,class FSM,class SourceState,class TargetState>
                bool operator()(EVT const& evt ,FSM&,SourceState& ,TargetState& ) {             
                    return true;
                }
            };

            typedef controller_ p; // makes transition table cleaner

            // Transition table
            struct transition_table : mpl::vector<
                //    Start                    Event             Next                    Action				 Guard
                //  +-------------------------+------------------+----------------------+---------------------+----------------------+
                Row   < Stopped,               Stop,              Stopped,               none,             none >
                , Row < Stopped,               Start,             PreparingForRun,       actStart,         none >
                , Row < Stopped,               Ready,             WaitForContactClosure, actReady,         none >
                , Row < PreparingForRun,       Start,             PreparingForRun,       none,             none >
                , Row < PreparingForRun,       Ready,             WaitForContactClosure, actReady,         none >
                , Row < WaitForContactClosure, Stop,              Stopped,               actStop,          none >
                , Row < WaitForContactClosure, Inject,            Running,               actInject,        none >
                , Row < Running,               Stop,              Dormant,               actComplete,      none >
                , Row < Running,               Complete,          Dormant,               actComplete,      none >
                , Row < Running,               Start,             PreparingForRun,       actStart,         none >
                , Row < Dormant,               Stop,              Stopped,               actStop,          none >
                , Row < Dormant,               Start,             PreparingForRun,       actStart,         none >
                , Row < Dormant,               Ready,             WaitForContactClosure, actReady,         none >
                > {};

            // Replaces the default no-transition response.
            template <class FSM,class Event> void no_transition(Event const& e, FSM& fsm, int state)  {
                fsm.handler_->fsm_no_transition( state );
            }

            template <class FSM,class Event> void exception_caught(Event const& ev, FSM& fsm, std::exception& ex) {
                fsm.handler_->fsm_exception_caught( typeid(ev).name(), ex );
            }

            handler* handler_;
        };

        // handler
        struct handler {
            virtual void fsm_action_stop() = 0;
            virtual void fsm_action_start() = 0;
            virtual void fsm_action_ready() = 0;
            virtual void fsm_action_inject() = 0;
            virtual void fsm_action_complete() = 0;
            virtual void fsm_state( bool, idState, int id_state = 0 ) = 0;
            virtual void fsm_no_transition( int state ) = 0;
            virtual void fsm_exception_caught( const char *, const std::exception& ) = 0;
        };

        // Pick a back-end
        typedef msm::back::state_machine< controller_ > controller;
    }
}
