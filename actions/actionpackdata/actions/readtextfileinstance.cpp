/*
		Actionaz
		Copyright (C) 2008-2010 Jonathan Mercier-Ganady

		Actionaz is free software: you can redistribute it and/or modify
		it under the terms of the GNU General Public License as published by
		the Free Software Foundation, either version 3 of the License, or
		(at your option) any later version.

		Actionaz is distributed in the hope that it will be useful,
		but WITHOUT ANY WARRANTY; without even the implied warranty of
		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
		GNU General Public License for more details.

		You should have received a copy of the GNU General Public License
		along with this program. If not, see <http://www.gnu.org/licenses/>.

		Contact : jmgr@jmgr.info
*/

#include "readtextfileinstance.h"
#include "actioninstanceexecutionhelper.h"

#include <QFile>
#include <QTextStream>

namespace Actions
{
	ActionTools::StringListPair ReadTextFileInstance::modes = qMakePair(
			QStringList() << "full" << "selection",
			QStringList() << QObject::tr("Read the entire file") << QObject::tr("Read only a selection"));

	void ReadTextFileInstance::startExecution()
	{
		ActionTools::ActionInstanceExecutionHelper actionInstanceExecutionHelper(this, script(), scriptEngine());
		QString filepath;
		QString variable;
		Mode mode;
		int firstline;
		int lastline;

		if(!actionInstanceExecutionHelper.evaluateString(filepath, "file") ||
		   !actionInstanceExecutionHelper.evaluateVariable(variable, "variable") ||
		   !actionInstanceExecutionHelper.evaluateListElement(mode, modes, "mode") ||
		   !actionInstanceExecutionHelper.evaluateInteger(firstline, "firstline") ||
		   !actionInstanceExecutionHelper.evaluateInteger(lastline, "lastline"))
			return;

		if(mode == Selection)
		{
			if(firstline < 1)
			{
				actionInstanceExecutionHelper.setCurrentParameter("firstline");
				emit executionException(ActionTools::ActionException::BadParameterException, tr("Invalid first line value : %1").arg(firstline));
				return;
			}

			if(lastline < 1)
			{
				actionInstanceExecutionHelper.setCurrentParameter("lastline");
				emit executionException(ActionTools::ActionException::BadParameterException, tr("Invalid last line value : %1").arg(lastline));
				return;
			}

			if(lastline < firstline)
			{
				actionInstanceExecutionHelper.setCurrentParameter("firstline");
				emit executionException(ActionTools::ActionException::BadParameterException, tr("The first line has to be smaller than the last line"));
				return;
			}
		}

		QFile file(filepath);
		if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			actionInstanceExecutionHelper.setCurrentParameter("file");
			emit executionException(CannotOpenFileException, tr("Cannot open file"));
			return;
		}

		//Line counting starts at 1
		--firstline;
		--lastline;

		QString result;

		if(mode == Full)
			result = file.readAll();
		else
		{
			QTextStream stream(&file);

			for(int line = 0; !stream.atEnd(); ++line)
			{
				QString readLine = stream.readLine();

				if(line >= firstline && line <= lastline)
				{
					if(!result.isEmpty())
						result += '\n';

					result += readLine;
				}

				if(line > lastline)
					break;
			}
		}

		actionInstanceExecutionHelper.setVariable(variable, result);

		emit executionEnded();
	}
}