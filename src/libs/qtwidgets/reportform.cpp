//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "reportform.h"
#include "ui_reportform.h"
#include "reportdelegate.h"
#include <adcontrols/reportmethod.h>
#include <adportable/configuration.h>
#include <QStandardItemModel>

using namespace qtwidgets;

ReportForm::ReportForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReportForm)
    , pModel_( new QStandardItemModel )
    , pConfig_( new adportable::Configuration )
    , pDelegate_( new ReportDelegate )
    , pMethod_( new adcontrols::ReportMethod )
{
    ui->setupUi(this);
    ui->treeView->setModel( pModel_.get() );
}

ReportForm::~ReportForm()
{
    delete ui;
}

void
ReportForm::OnCreate( const adportable::Configuration& config )
{
    *pConfig_ = config;
}

void
ReportForm::OnInitialUpdate()
{
    QStandardItemModel& model = *pModel_;
    adcontrols::ReportMethod& method = *pMethod_;

    QStandardItem * rootNode = model.invisibleRootItem();

    ui->treeView->setItemDelegate( pDelegate_.get() );

    rootNode->setColumnCount(2);
    model.setHeaderData( 0, Qt::Horizontal, "Report" );
}

void
ReportForm::OnFinalClose()
{
}
