/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express oqr
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is the JavaScript 2 Prototype.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 *
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU Public License (the "GPL"), in which case the
 * provisions of the GPL are applicable instead of those above.
 * If you wish to allow use of your version of this file only
 * under the terms of the GPL and not to allow others to use your
 * version of this file under the NPL, indicate your decision by
 * deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL.  If you do not delete
 * the provisions above, a recipient may use your version of this
 * file under either the NPL or the GPL.
 */

#include <assert.h>
#include "world.h"
#include "parser.h"
#include "reader.h"

using namespace JavaScript;

World world;
const bool showTokens = false;
const String consoleName = widenCString("<console>");

#if defined(XP_MAC) && !defined(XP_MAC_MPW)
#include <SIOUX.h>
#include <MacTypes.h>

static char *mac_argv[] = {"js2", 0};

static void initConsole(StringPtr consoleName, const char* startupMessage,
                        int &argc, char **&argv)
{
    SIOUXSettings.autocloseonquit = false;
    SIOUXSettings.asktosaveonclose = false;
    SIOUXSetTitle(consoleName);

    // Set up a buffer for stderr (otherwise it's a pig).
    static char buffer[BUFSIZ];
    setvbuf(stderr, buffer, _IOLBF, BUFSIZ);

    JavaScript::stdOut << startupMessage;

    argc = 1;
    argv = mac_argv;
}
#endif

// Interactively read a line from the input stream in and put it into
// s. Return false if reached the end of input before reading anything.
static bool promptLine(LineReader &inReader, string &s, const char *prompt)
{
#if !defined XP_MAC_MPW
    if (prompt)
        stdOut << prompt;
#endif
    return inReader.readLine(s) != 0;
}

static void readEvalPrint(FILE *in, World &world)
{
    String buffer;
    string line;
    LineReader inReader(in);

    while (promptLine(inReader, line, buffer.empty() ? "js parser> " : "> ")) {
        appendChars(buffer, line.data(), line.size());
        try {
            Arena a;
            Parser p(world, a, buffer, consoleName);
                
            if (showTokens) {
                Lexer &l = p.lexer;
                while (true) {
                    const Token &t = l.get(true);
                    if (t.hasKind(Token::end))
                        break;
                    stdOut << ' ';
                    t.print(stdOut, true);
                }
	            stdOut << '\n';
            } else {
                StmtNode *parsedStatements = p.parseProgram();
				ASSERT(p.lexer.peek(true).hasKind(Token::end));
                {
                	PrettyPrinter f(stdOut, 30);
                	{
                		PrettyPrinter::Block b(f, 2);
	                	f << "Program =";
	                	f.linearBreak(1);
	                	StmtNode::printStatements(f, parsedStatements);
                	}
                	f.end();
                }
        	    stdOut << '\n';
            }
            clear(buffer);
        } catch (Exception &e) {
            /* If we got a syntax error on the end of input,
             * then wait for a continuation
             * of input rather than printing the error message. */
            if (!(e.hasKind(Exception::syntaxError) &&
                  e.lineNum && e.pos == buffer.size() &&
                  e.sourceFile == consoleName)) {
                stdOut << '\n' << e.fullMessage();
                clear(buffer);
            }
        }
    }
    stdOut << '\n';
}


#if defined(XP_MAC) && !defined(XP_MAC_MPW)
int main(int argc, char **argv)
{
    initConsole("\pJavaScript Shell", "Welcome to the js2 shell.\n", argc, argv);
#else
int main(int , char **)
{
#endif

    readEvalPrint(stdin, world);
    
    return 0;
    // return ProcessArgs(argv + 1, argc - 1);
}
