//
//  renderoptions.h
//  yaluxplug
//
//  Created by Daniel Bui on 4/24/15.
//
//

#ifndef __yaluxplug__optionsframe__
#define __yaluxplug__optionsframe__

#include <dzoptionsframe.h>

class DzFilterNavigationBar;
class DzSideNavPropertyListView;
class DzFileProperty;
class DzStringProperty;
class DzAppSettings;

class YaLuxOptionsFrame : public DzOptionsFrame {
    Q_OBJECT
public:
//    YaLuxOptions(const QString &label, QWidget *parent=0, const QString &name=QString::null);
    YaLuxOptionsFrame();

public slots:    
    virtual void	applyChanges();
    virtual void	resetOptions();
    virtual bool	applyValid() const;
    virtual void	restoreOptions( DzRenderOptions *options );
    
signals:
    void	optionsChanged();


public:
    DzAppSettings *settings;

private:
    void loadSavedSettings();


private:
    DzFilterNavigationBar *filterBar;
    DzSideNavPropertyListView *listView;
    DzFileProperty *fileProperty;
    DzStringProperty *argumentsProperty;


};


#endif // __yaluxplug__optionsframe__
