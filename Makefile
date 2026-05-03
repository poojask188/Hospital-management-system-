# Hospital Management System - Makefile
# Run: make
# Then: make install  (copies to Apache CGI folder)

CC      = gcc
CFLAGS  = -Wall -Wextra -O2
LIBS    = -lsqlite3
TARGET  = hospital.cgi
SRC     = hospital.c
DB      = hospital.db
CGI_DIR = /usr/lib/cgi-bin

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)
	@echo "✅ Compiled: $(TARGET)"

db: $(DB)

$(DB): database.sql
	sqlite3 $(DB) < database.sql
	@echo "✅ Database created: $(DB)"

install: $(TARGET) $(DB)
	sudo cp $(TARGET) $(CGI_DIR)/$(TARGET)
	sudo cp $(DB) $(CGI_DIR)/$(DB)
	sudo chmod 755 $(CGI_DIR)/$(TARGET)
	sudo chmod 666 $(CGI_DIR)/$(DB)
	@echo "✅ Installed to $(CGI_DIR)"

clean:
	rm -f $(TARGET) $(DB)
	@echo "🧹 Cleaned"

run: all db
	@echo "🏥 Starting hospital CGI server..."
	@echo "Open: http://localhost/cgi-bin/hospital.cgi?action=list"

.PHONY: all db install clean run
