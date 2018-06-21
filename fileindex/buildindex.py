'''
buildindex module allows to build a word index database of given directory
'''
import base64
import md5
import os
import re
import sqlite3
import stat

class WordsDatabase(object):
    '''
    Provides access to the words database
    '''
    TABLE_DEFS = {
        "words": "create table words(ID INTEGER PRIMARY KEY AUTOINCREMENT, Word text not null)",
        "files": ("create table files(ID INTEGER PRIMARY KEY AUTOINCREMENT, "
                  "FileName text, Modified int, Hash text)"),
        "locations": ("create table locations(ID INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "FileID int, Line int, column int, "
                      "constraint locs_in_file "
                      "foreign key (FileID) REFERENCES files(ID) on delete cascade)"),
        "occurrences": ("create table occurrences(WordID int, LocId int, "
                        "constraint word_occurred "
                        "foreign key (WordID) references words(ID) on delete cascade, "
                        "constraint occurred_at_loc "
                        "foreign key (LocID) references locations(ID) on delete cascade)")}

    INDEX_DEFS = [
        "create index IX_Occ_Words on occurrences (WordID)",
        "create index IX_occ_locs on occurrences (LocID)"
    ]

    # index for info fields
    F_ID = 0
    F_FILE_NAME = 1
    F_MTIME = 2
    F_HASH = 3


    WORDS_RE = re.compile(r"\w+", re.LOCALE|re.UNICODE)
    __slots__ = ('__cursor', '__db_name')

    def __init__(self, database_name="words.sqlite"):
        self.__db_name = database_name
        conn = sqlite3.connect(self.__db_name)
        self.__cursor = conn.cursor()


    def create_database(self):
        '''
        (Re-)creates a database file on a disk
        '''
        self.__cursor.connection.close()
        self.__cursor = None
        if os.path.exists(self.__db_name):
            os.unlink(self.__db_name)
        conn = sqlite3.connect(self.__db_name)

        self.__cursor = conn.cursor()

        for table_def in self.TABLE_DEFS.itervalues():
            self.__cursor.execute(table_def)

        for index_def in self.INDEX_DEFS:
            self.__cursor.execute(index_def)

    def check_database(self):
        '''
        Checks if the database file exists and is valid
        '''
        table_names = set(x[0].lower() for x in
                          self.__cursor.execute('select name from sqlite_master '
                                                'where type = "table"'))
        return table_names == set(self.TABLE_DEFS.iterkeys()) | set(['sqlite_sequence'])

    def get_word_id(self, word):
        '''
        Returns an id of the specified word
        '''
        word = word.lower()
        result = [x for x in self.__cursor.execute('select id from words where word = ?', (word,))]
        if len(result) > 1:
            raise Exception("Word {0} is found more than once", word)
        elif len(result) < 1:
            self.__cursor.execute('insert into words (word) values (?)', (word,))
            self.__cursor.connection.commit()
            return self.__cursor.lastrowid
        else:
            return result[0][0]

    def get_file_info(self, file_path):
        """
        Returns a tuple of form (id, file_path, modification_time, hash)
        """
        file_info = [x for x in self.__cursor.execute(
            'select id, filename, modified, hash from files where filename = ?', (file_path,))]
        if len(file_info) > 1:
            raise Exception("Found duplicated record for {0} in files table".format(file_path))
        elif len(file_info) < 1:
            self.__cursor.execute('insert into files (filename) values (?)', (file_path,))
            self.__cursor.connection.commit()
            return (self.__cursor.lastrowid, file_path, 0, "")
        else:
            return file_info[0]

    def update_file_info(self, file_id, mtime, md5hash):
        '''
        Updates existing file info in the database
        '''
        self.__cursor.execute('update files set modified = ?2, hash = ?3 where id = ?1',
                              (file_id, mtime, md5hash))
        self.__cursor.connection.commit()

    def process_file(self, file_path):
        '''
        Processes a single file in the database
        '''
        info = self.get_file_info(file_path)
        file_stat = os.stat(file_path)
        if info[self.F_MTIME] != file_stat[stat.ST_MTIME]:
            with open(file_path, 'r') as file_object:
                file_content = file_object.read()
            md5hash = md5.new()
            md5hash.update(file_content)
            md5hash = base64.b64encode(md5hash.digest())
            if md5hash != info[self.F_HASH]:
                self.process_file_content(info[self.F_ID], file_content)
            self.update_file_info(info[self.F_ID], file_stat[stat.ST_MTIME], md5hash)
        else:
            print "file has not been changed"

    def delete_file_locations(self, file_id):
        '''
        Deletes entries in the locations table
        '''
        self.__cursor.execute('delete from locations where fileid = ?', (file_id,))
        self.__cursor.connection.commit()

    def add_file_location(self, file_id, line, column, commit=False):
        '''
        Adds file location to the appropriate file
        '''
        self.__cursor.execute('insert into locations (fileid, line, column) '
                              'values (?, ?, ?)', (file_id, line, column))
        if commit:
            self.__cursor.connection.commit()
        return self.__cursor.lastrowid

    def add_word_occurrence(self, word_id, location_id, commit=False):
        '''
        Adds a single word occurrence
        '''
        self.__cursor.execute('insert into occurrences (wordid, locid) values (?, ?)',
                              (word_id, location_id))
        if commit:
            self.__cursor.connection.commit()
        return self.__cursor.lastrowid

    def process_file_content(self, file_id, file_content):
        '''
        Processes content of a single file
        '''
        self.delete_file_locations(file_id)
        for i, line in enumerate(file_content.splitlines()):
            for match in self.WORDS_RE.finditer(line):
                word_id = self.get_word_id(match.group())
                self.add_word_occurrence(word_id,
                                         self.add_file_location(file_id, i + 1, match.start() + 1))
        self.__cursor.connection.commit()


def main():
    '''
    The entry point if the module is used as a script
    '''
    words_db = WordsDatabase()

    if not words_db.check_database():
        words_db.create_database()

    for (dirpath, _, filenames) in os.walk('.'):
        for filename in filenames:
            if filename.endswith('.vb'):
                print filename
                filename = os.path.abspath(os.path.join(dirpath, filename))
                words_db.process_file(filename)

if __name__ == '__main__':
    main()

# vim: set et ts=4 sw=4 ai :
