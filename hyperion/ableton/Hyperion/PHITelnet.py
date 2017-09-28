import sys
sys.path.append('/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7')
import StringIO, socket, code


# BASED ON https://forum.ableton.com/viewtopic.php?p=490097&sid=26f235189033abd1c85684fa08a10e41
class PHITelnetMixin(object):
    def __init__(self, *args, **kwargs):
        super(PHITelnetMixin, self).__init__(*args, *kwargs)

        self.originalstdin = sys.stdin
        self.originalstdout = sys.stdout
        self.originalstderr = sys.stderr

        self.stdin = StringIO.StringIO()
        self.stdout = StringIO.StringIO()
        self.stderr = StringIO.StringIO()

        self.telnetSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.telnetSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.telnetSocket.bind( ('', 1337) )
        self.telnetSocket.setblocking(False)
        self.telnetSocket.listen(1)
        self.telnetConnection = None

        self.interpreter = code.InteractiveConsole(globals())

        self.telnetBuffer = ""
        self.lastData = ""
        self.commandBuffer = []

    def disconnect(self):
        super(PHITelnetMixin, self).disconnect()

        sys.stdin = self.originalstdin
        sys.stdout = self.originalstdout
        sys.stderr = self.originalstderr
        self.telnetSocket.close()

    def update_display(self):
        super(PHITelnetMixin, self).update_display()

        #Keep trying to accept a connection until someone actually connects
        if not self.telnetConnection:
            try:
                #Does anyone want to connect?
                self.telnetConnection, self.addr = self.telnetSocket.accept()
            except:
                #No one connected in this iteration
                pass
            else:
                #Yay! Someone connected! Send them the banner and first prompt.
                self.telnetConnection.send("Welcome to the Ableton Live Python Interpreter\r\n")
                self.telnetConnection.send("Brought to by LiveAPI.org\r\n")
                self.telnetConnection.send(">>> ")
        else:
            #Someone's connected, so lets interact with them.
            try:
                #If the client has typed anything, get it
                data = self.telnetConnection.recv(1)
            except:
                #Nope they haven't typed anything yet
                data = "" #

            #If return is pressed, process the command (This if statement is so ugly because ableton python doesn't have universal newlines)
            if (data == "\n" or data == "\r") and (self.lastData != "\n" and self.lastData != "\r"):
                continues = self.interpreter.push(self.telnetBuffer.rstrip()) #should be strip("/r/n") but ableton python throws an error
                self.commandBuffer.append(self.telnetBuffer.rstrip())
                self.telnetBuffer = ""

                #if the user input is multi-line, continue, otherwise return the results

                if continues:
                    self.telnetConnection.send("... ")
                else:
                    #return stdout to the client
                    self.telnetConnection.send(self.stdout.getvalue().replace("\n","\r\n"))
                    #return stderr to the client
                    self.telnetConnection.send(self.stderr.getvalue().replace("\n","\r\n"))
                    self.telnetConnection.send(">>> ")

                #Empty buffers by creating new stringIO objects
                #There's probably a better way to empty these
                self.stdin.close()
                self.stdout.close()
                self.stderr.close()
                self.stdin = StringIO.StringIO()
                self.stdout = StringIO.StringIO()
                self.stderr = StringIO.StringIO()
                #re-redirect the stdio
                sys.stdin = self.stdin
                sys.stdout = self.stdout
                sys.stderr = self.stderr


            elif data == "\b": #deals with backspaces
                if len(self.telnetBuffer):
                    self.telnetBuffer = self.telnetBuffer[:-1]
                    self.telnetConnection.send(" \b") #deletes the character on the console
                else:
                    self.telnetConnection.send(" ")
            elif data != "\n" and data != "\r":
                self.telnetBuffer = self.telnetBuffer + data
            self.lastData = data
