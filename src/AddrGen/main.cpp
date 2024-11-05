#include <iostream>
#include<string>
#include "secp256k1.h"
#include "util.h"
#include "AddressUtil.h"
#include "CmdParse.h"

/**
 * Parses the start:end key pair. Possible values are:
 start
 start:end
 start:+offset
 :end
 :+offset
 */
bool parseKeyspace(const std::string &s, secp256k1::uint256 &start, secp256k1::uint256 &end)
{
    size_t pos = s.find(':');

    if(pos == std::string::npos) {
        start = secp256k1::uint256(s);
        end = secp256k1::N - 1;
    } else {
        std::string left = s.substr(0, pos);

        if(left.length() == 0) {
            start = secp256k1::uint256(1);
        } else {
            start = secp256k1::uint256(left);
        }

        std::string right = s.substr(pos + 1);

        if(right[0] == '+') {
            end = start + secp256k1::uint256(right.substr(1));
        } else {
            end = secp256k1::uint256(right);
        }
    }

    return true;
}

int main(int argc, char **argv)
{
    std::vector<secp256k1::uint256> keys;

	bool compressed = true;
    bool printPrivate = false;
    bool printPublic = false;
    bool printAddr = false;
    bool printAll = true;
    int count = 1;
    bool useKeyspace = false;
    secp256k1::uint256 min(1);
    secp256k1::uint256 max(secp256k1::N);
    secp256k1::uint256 ONE(1);

	// secp256k1::uint256 k;

	// k = secp256k1::generatePrivateKey();

	CmdParse parser;

	parser.add("-c", "--compressed", false);
	parser.add("-u", "--uncompressed", false);
    parser.add("-p", "--pub", false);
    parser.add("-k", "--priv", false);
    parser.add("-a", "--addr", false);
    parser.add("", "--keyspace", true);
    parser.add("-n", true);

	parser.parse(argc, argv);

	std::vector<OptArg> args = parser.getArgs();

	for(unsigned int i = 0; i < args.size(); i++) {
		OptArg arg = args[i];
		
		if(arg.equals("-c", "--compressed")) {
			compressed = true;
		} else if(arg.equals("-u", "--uncompressed")) {
			compressed = false;
        } else if(arg.equals("-k", "--priv")) {
            printAll = false;
            printPrivate = true;
        } else if(arg.equals("-p", "--pub")) {
            printAll = false;
            printPublic = true;
        } else if(arg.equals("-a", "--addr")) {
            printAll = false;
            printAddr = true;
        } else if(arg.equals("-n")) {
            count = (int)util::parseUInt32(arg.arg);
        } else if(arg.equals("", "--keyspace")) {
            secp256k1::uint256 start;
            secp256k1::uint256 end;

            parseKeyspace(arg.arg, start, end);

            if(start.cmp(secp256k1::N) > 0) {
                printf("Error parsing keyspace: start out of range\n");
                return 1;
            }
            if(start.isZero()) {
                printf("Error parsing keyspace: start out of range\n");
                return 1;
            }

            if(end.cmp(secp256k1::N) > 0) {
                printf("Error parsing keyspace: end out of range\n");
                return 1;
            }

            if(start.cmp(end) > 0) {
                printf("Error parsing keyspace: start larger than end\n");
                return 1;
            }

            useKeyspace = true;
            min = start;
            max = end+ONE;
        }
	}

	std::vector<std::string> operands = parser.getOperands();

	if(operands.size() > 0) {
        for(int i = 0; i < operands.size(); i++) {
            try {
                keys.push_back(secp256k1::uint256(operands[i]));
            } catch(std::string err) {
                printf("Error parsing private key: %s\n", err.c_str());
                return 1;
            }
        }
    } else {
        for(int i = 0; i < count; i++) {
            if(useKeyspace) {
                keys.push_back(secp256k1::generatePrivateKey(min, max));
            } else {
                keys.push_back(secp256k1::generatePrivateKey());
            }
        }
    }

    for(int i = 0; i < keys.size(); i++) {
        secp256k1::uint256 k = keys[i];

        if(k.isZero() || k.cmp(secp256k1::N) >= 0)
        {
            printf("Error parsing private key: Private key is out of range\n");

            return 1;
        }

        secp256k1::ecpoint p = secp256k1::multiplyPoint(k, secp256k1::G());
        std::string address = Address::fromPublicKey(p, compressed);

        if(printAll || printPrivate) {
            std::cout << k.toString() << std::endl;
        }
        if(printAll || printPublic) {
            std::cout << p.toString() << std::endl;
        }
        if(printAll || printAddr) {
            std::cout << address << std::endl;
        }
    }

	return 0;
}