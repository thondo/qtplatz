//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "analysisplugin.h"
#include <QtCore/qplugin.h>
#include <coreplugin/icore.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <QStringList>

using namespace Analysis;
using namespace Analysis::internal;

AnalysisPlugin::~AnalysisPlugin()
{
}

AnalysisPlugin::AnalysisPlugin()
{
}

bool
AnalysisPlugin::initialize(const QStringList& arguments, QString* error_message)
{
  Q_UNUSED( arguments );
  Q_UNUSED( error_message );
  return true;
}


void
AnalysisPlugin::extensionsInitialized()
{
}

Q_EXPORT_PLUGIN( AnalysisPlugin )
