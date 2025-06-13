
namespace BackHttpUtility
{

	typedef unsigned char BYTE;

	BYTE toHex(const BYTE &x);

	BYTE fromHex(const BYTE &x);

	string URLEncode(const string &sIn);

	string URLDecode(const string &sIn);
}
