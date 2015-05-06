//
//  renderoptions.h
//  yaluxplug
//
//  Created by Daniel Bui on 4/24/15.
//
//

#ifndef __yaluxplug__renderoptions__
#define __yaluxplug__renderoptions__

#include <dzoptionsframe.h>

class YaLuxRenderOptions : public DzOptionsFrame {
    Q_OBJECT
public:
//    YaLuxRenderOptions(const QString &label, QWidget *parent=0, const QString &name=QString::null);
    YaLuxRenderOptions();

public slots:    
    virtual void	applyChanges();
    virtual void	resetOptions();
    virtual bool	applyValid() const;
    virtual void	restoreOptions( DzRenderOptions *options );
    
signals:
    void	optionsChanged();
        
};


#endif // __yaluxplug__renderoptions__
