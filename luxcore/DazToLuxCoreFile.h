#pragma once
#include "DazToRenderFile.h"
class DazToLuxCoreFile : public DazToRenderFile
{
public:
	DazToLuxCoreFile(DzRenderer* r, DzCamera* c, const DzRenderOptions& opt, QString filenameBase) :
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
	QString renderfilenameEXT = "CFG";

};
