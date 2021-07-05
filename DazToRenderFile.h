#pragma once
#include <qfile.h>
#include "dzrenderer.h"
#include "dzcamera.h"
#include "dzrenderoptions.h"

class DazToRenderFile
{
public:
	DazToRenderFile(DzRenderer* r, DzCamera* c, const DzRenderOptions& opt, QString filenameBase)
	{
		renderer = r;
		camera = c;
		options.copyFrom( &opt);
		fullPathRenderFilenameBase = filenameBase;

	}

	virtual bool WriteRenderFiles() = 0;
	virtual QString GetFullPathRenderFilename() = 0;
	virtual QString GetFilenameExtension() = 0;
	virtual QStringList GetListFullPathRenderFilenames() = 0;
	virtual QStringList GetListFilenameExtensions() = 0;

protected:
	DzRenderer* renderer;
	DzCamera* camera;
	DzRenderOptions options;
	QString fullPathRenderFilenameBase;

};
