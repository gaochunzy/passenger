/*
 *  Phusion Passenger - https://www.phusionpassenger.com/
 *  Copyright (c) 2010-2014 Phusion
 *
 *  "Phusion Passenger" is a trademark of Hongli Lai & Ninh Bui.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 */

class HelperAgentWatcher: public AgentWatcher {
protected:
	string agentFilename;

	virtual const char *name() const {
		return PROGRAM_NAME " helper agent";
	}

	virtual string getExeFilename() const {
		return agentFilename;
	}

	virtual void execProgram() const {
		if (hasEnvOption("PASSENGER_RUN_HELPER_AGENT_IN_VALGRIND", false)) {
			execlp("valgrind", "valgrind", "--dsymutil=yes", "--track-origins=yes", "--leak-check=full",
				agentFilename.c_str(), "server",
				// Some extra space to allow the child process to change its process title.
				"                                                ", (char *) 0);
		} else {
			execl(agentFilename.c_str(), AGENT_EXE, "server",
				// Some extra space to allow the child process to change its process title.
				"                                                ", (char *) 0);
		}
	}

	virtual void sendStartupArguments(pid_t pid, FileDescriptor &fd) {
		VariantMap options = *agentsOptions;
		options.erase("logging_agent_authorizations");
		options.writeToFd(fd);
	}

	virtual bool processStartupInfo(pid_t pid, FileDescriptor &fd, const vector<string> &args) {
		return args[0] == "initialized";
	}

public:
	HelperAgentWatcher(const WorkingObjectsPtr &wo)
		: AgentWatcher(wo)
	{
		agentFilename = wo->resourceLocator->findSupportBinary(AGENT_EXE);
	}

	virtual void reportAgentsInformation(VariantMap &report) {
		const VariantMap &options = *agentsOptions;
		vector<string> addresses = options.getStrSet("server_addresses");
		report.set("server_address", addresses.front());
		report.set("server_password", options.get("server_password"));
	}
};
