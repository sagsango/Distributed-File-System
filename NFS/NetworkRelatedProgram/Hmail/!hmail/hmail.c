/*
    hmail
    Copyright © Alex Waugh 2005

    $Id$

    Simple command line program that sends mail to an SMTP relay.


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include <stdio.h>
#include <syslog.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>



#define BUFFER_SIZE 4096

int sock = -1;

static void error(char *msg, ...)
{
   va_list ap;

   va_start(ap, msg);
   vsyslog(1, msg, ap);
   va_end(ap);

   if (sock != -1) close(sock);

   exit(EXIT_FAILURE);
}

static char *readline(void)
{
    static char buf[1024] = "";
    static char buf2[1024];
    static int len = 0;
    int ret;
    char *crlf;

    do {
        crlf = strstr(buf, "\r\n");

        if (crlf) {
            int linelen = crlf - buf;
            memcpy(buf2, buf, linelen);
            buf2[linelen] = '\0';
            memmove(buf, buf + linelen + 2, len - linelen - 2);
            len -= linelen + 2;
            buf[len] = '\0';
            return buf2;
        }

        ret = read(sock, buf + len, sizeof(buf) - 1 - len);
        if (ret == EOF) error("Read from socket failed (%s)",strerror(errno));

        len += ret;
        buf[len] = '\0';
    } while (ret > 0);

    error("No data reurned from socket read");

    return NULL;
}

static void getresponse(int code)
{
    char *line;
    int cont;
    int num;

    do {
        line = readline();
        num = strtol(line, &line, 10);
    } while (*line == '-');

    if ((num - code) < 0 || (num - code) >= 100) {
        error("Incorrect SMTP response recieved '%s'", line);
    }
}

static char *extractaddress(char *line)
{
    char *address;
    char *end = line;
    char *start;
    char *addr;

    while (*end >= ' ') end++;

    address = malloc((end - line) + 1);
    if (address == NULL) error("Out of memory");

    addr = address;

    while (line < end && *line != '>') {
        if (*line == '(') {
            while (line < end && *line != ')') line++;
            if (*line == ')') line++;
        } else if (*line == '<') {
            addr = address;
            line++;
        } else if (*line != ' ') {
            *addr++ = *line++;
        } else {
            line++;
        }
    }
    *addr = '\0';

    return address;
}

static char *getline(char *buffer, size_t buflen, size_t *bufpos)
{
    char *line = buffer + *bufpos;

    while (*bufpos + 1 < buflen) {
        if (buffer[*bufpos] == '\n') {
            (*bufpos)++;
            return line;
        }
        if (buffer[*bufpos] == '\r' && buffer[*bufpos + 1] == '\n') {
            (*bufpos) += 2;
            return line;
        }
        (*bufpos)++;
    }
    return NULL;
}

static void writedata(char *fmt, ...)
{
   char buffer[BUFFER_SIZE];
   va_list ap;

   va_start(ap, fmt);
   vsnprintf(buffer, sizeof(buffer), fmt, ap);
   va_end(ap);

   if (write(sock, buffer, strlen(buffer)) == EOF) {
      error("Write to socket failed (%s)", strerror(errno));
   }
}

static char *getsettings(int *port)
{
    static char line[1024];
    char *p;
    FILE *file = fopen("Choices:hmail", "r");
    if (file == NULL) error("Cannot open settings file Choices:hmail");

    if (fgets(line, sizeof(line), file) == NULL) error("Cannot read line from choices file");
    p = strchr(line, ':');
    if (p) {
        *p++ = '\0';
        *port = atoi(p);
    } else {
        p = strchr(line, '\n');
        if (p) *p++ = '\0';
        *port = 25;
    }

    fclose(file);

    return line;
}

static void setsettings(char *server)
{
    char *filename = "<Choices$Write>.hmail";
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Cannot open settings file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    if (fprintf(file, "%s\n", server) == EOF) {
        fprintf(stderr, "Cannot write to setting file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    fclose(file);
}

int main(int argc, char **argv)
{
    char buffer[BUFFER_SIZE];
    size_t buflen;
    size_t bufpos;
    char *to = NULL;
    char *from = NULL;
    char *smtpserver;
    int smtpport;

    if (argc == 3 && strcmp(argv[1], "--server") == 0) {
        setsettings(argv[2]);
        return EXIT_SUCCESS;
    }

    openlog("hmail", 0, LOG_MAIL);

    smtpserver = getsettings(&smtpport);

    do {
        buflen = fread(buffer, 1, BUFFER_SIZE, stdin);
        bufpos = 0;

        while (to == NULL || from == NULL) {
            char *line = getline(buffer, buflen, &bufpos);

            if (line == NULL) {
               error("%s header not found in first %d bytes of email", to ? "TO" : "FROM", BUFFER_SIZE);
            }

            if (strncasecmp(line, "To:", 3) == 0) {
                to = extractaddress(line + 3);
            } else if (strncasecmp(line, "From:", 5) == 0) {
                from = extractaddress(line + 5);
            }
        }

        bufpos = 0;
        if (sock == -1) {
            char *hostname;
            char *domain;
            struct hostent *hp;
            struct sockaddr_in sockaddr;

            hostname = getenv("Inet$Hostname");
            if (hostname) hostname = strdup(hostname);
            domain = getenv("Inet$LocalDomain");
            if (domain) domain = strdup(domain);

            sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock == EOF) {
                error("Unable to open socket (%s)", strerror(errno));
            }

            hp = gethostbyname(smtpserver);
            if (hp == NULL) error("Unable to resolve %s (%s)", smtpserver, strerror(errno));

            memset(&(sockaddr), 0, sizeof(sockaddr));
            memcpy(&(sockaddr.sin_addr), hp->h_addr, hp->h_length);
            sockaddr.sin_family = AF_INET;
            sockaddr.sin_port = htons(smtpport);

            if (connect(sock, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr_in)) == EOF) {
                error("Unable to connect socket (%s)", strerror(errno));
            }

            getresponse(200);
            writedata("EHLO %s.%s\r\n",hostname ? hostname : "", domain ? domain : "");
            getresponse(200);
            writedata("MAIL FROM:<%s>\r\n",from);
            getresponse(200);
            writedata("RCPT TO:<%s>\r\n",to);
            getresponse(200);
            writedata("DATA\r\n");
            getresponse(300);
        }

        do {
            int written = write(sock, buffer + bufpos, buflen - bufpos);
            if (written == EOF) {
               error("Write to socket failed (%s)", strerror(errno));
            }
            bufpos += written;
         } while (bufpos < buflen);

    } while (buflen > 0);

    writedata("\r\n.\r\n");
    getresponse(200);
    writedata("QUIT\r\n");

    close(sock);

    syslog(10, "Sent mail to %s", to);

    return EXIT_SUCCESS;
}

