#ifndef UTILS_H
#define UTILS_H


int ArgPos(char *str, int argc, char **argv) {
	int a;
	for (a = 1; a < argc; a++) if (!strcmp(str, argv[a])) {
		if (a == argc - 1) {
			printf("Argument missing for %s\n", str);
			exit(1);
		}
		return a;
	}
	return -1;
}


class HUtils {

protected:
	
	

public:

	// Clean up objects with pointers stored in an STL container
	template<class Seq> static void purge(Seq& c) {
		typename Seq::iterator i;
		for(i = c.begin(); i != c.end(); ++i) {
			delete *i;
			*i = 0;
		}
	}
	
};

#endif	

