#pragma once
#include "DazToRenderFile.h"
class DazToLuxRenderFile : DazToRenderFile
{
public:
	DazToLuxRenderFile(DzRenderer* r, DzCamera* c, const DzRenderOptions& opt, QString filenameBase) :
		DazToRenderFile(r, c, opt, filenameBase)
	{
		renderfilename = filenameBase + "." + renderfilenameEXT;
	}

	bool WriteRenderFiles();
	QString GetFullPathRenderFilename() { return renderfilename; }
	QString GetFilenameExtension() { return renderfilenameEXT; }
	QStringList GetListFullPathRenderFilenames() { return QStringList(renderfilename); }
	QStringList GetListFilenameExtensions() { return QStringList(renderfilenameEXT); }

protected:
	QString renderfilename;
	QString renderfilenameEXT = "LXS";

};

