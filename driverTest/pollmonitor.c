
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define FIFO_CLEAR	0x1

int main()
{
	int fd;
	fd_set rfds, wfds;
	int ret;

	fd = open("/dev/globalfifo", O_RDONLY | O_NONBLOCK);
	if (fd != -1) {
		/**
		ret = ioctl(fd, FIFO_CLEAR, 0);
		if (ret < 0) {
			printf("ioctl command failed. %d\n", ret);
			return 1;
		}
		**/

		while (1) {
			FD_ZERO(&rfds);
			FD_ZERO(&wfds);
			FD_SET(fd, &rfds);
			FD_SET(fd, &wfds);

			select(fd + 1, &rfds, &wfds, NULL, NULL);
			// can be read
			if (FD_ISSET(fd, &rfds))
				printf("poll monitor: can be read.\n");

			// can be write
			if (FD_ISSET(fd, &wfds))
				printf("poll monitor: can be write.\n");
			
		}
	} else 
		printf("open device failure.\n");

	close(fd);

	return 0;
}
