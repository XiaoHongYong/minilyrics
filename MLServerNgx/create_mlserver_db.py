
import sqlite3
import os
import hashlib
import base64


def hex_to_str(hex_str):
    assert hex_str.startswith('0x')
    
    a = []
    for i in xrange(2, len(hex_str), 2):
        a.append(chr(int(hex_str[i:i + 2], 16)))
    
    return ''.join(a)


class DBLyrics:

    def __init__(self, conn_or_fn):
        if type(conn_or_fn) in (str, unicode):
            self._conn = sqlite3.connect(conn_or_fn, check_same_thread=False)
        else:
            self._conn = conn_or_fn
        cur = self._conn.cursor()

        self._conn.text_factory = str

        sql = '''CREATE TABLE IF NOT EXISTS lyrics (
        id integer primary key AUTOINCREMENT, content_type integer, media_length integer, 
        rate_total integer, rate_count integer, uploader_id integer, dl_count integer, 
        upload_time integer, artist text, artistcmp text, album text, title text, 
        titlecmp text, related_link text, edited_by text
        );'''
        # print fields_desc
        # print fields
        # print sql
        cur.execute(sql)

        index_fields = ['artistcmp', 'titlecmp']

        for name in index_fields:
            cur.execute(
                "CREATE INDEX IF NOT EXISTS lyrics_{0} on lyrics ({0});".format(name))

    def close(self):
        self._conn.close()
        self._conn = None

    def import_file(self, fn):
        fields = 'id,content_type,media_length,rate_total,rate_count,last_rate_ip,uploader_id,dl_count,upload_time,artist,artistcmp,album,title,titlecmp,related_link,edited_by'
        fields = fields.split(',')
        
        index_last_rate_ip = fields.index('last_rate_ip')
        assert(index_last_rate_ip != -1)
        fields.remove('last_rate_ip')

        text_fields = 'artist,artistcmp,album,title,titlecmp,related_link,edited_by'.split(',')
        index_text_fields = []
        for name in text_fields:
            index = fields.index(name)
            assert index != -1
            index_text_fields.append(index)
        
        count_fields = len(fields)


        sql = 'insert into lyrics ({}) values ({})'.format(','.join(fields), ','.join('?' * len(fields)))

        count = 0
        ignored_count = 0

        cur = self._conn.cursor()
        with open(fn, 'rb') as fp:
            fp.readline() # Ignore first line of format
            line = fp.readline()
            while line:
                line = line.strip()
                cols = line.split('\t')
                del cols[index_last_rate_ip]
                
                for index in index_text_fields:
                    cols[index] = hex_to_str(cols[index])

                if len(cols) == count_fields:
                    cur.execute(sql, cols)
                    count += 1
                    if count % 5000 == 0:
                        print 'Imported {} lines'.format(count)
                else:
                    ignored_count += 1
                    print 'Ignored {} line: {}'.format(ignored_count, line)

                line = fp.readline()

        self._conn.commit()
        
        cur.execute('select count(*) from lyrics')
        row = cur.fetchone()
        print 'Total records', row[0]
        print 'Lines:', count

    def dumpTopN(self, n):
        cur = self._conn.cursor()
        cur.execute('select * from lyrics limit {};'.format(n))

        names = list(map(lambda x: x[0], cur.description))
        print names

        while True:
            row = cur.fetchone()
            if row:
                values = []
                for i, col in enumerate(row):
                    values.append('{}: {}'.format(names[i], col))
                print ', '.join(values)
            else:
                break


SALT_PASSWORD = '@crintsoft.com'

def hash_password(password):
    return base64.b64encode(hashlib.md5(password + SALT_PASSWORD).digest())

def hash_minilyrics_client_password(password):
    digest = hashlib.md5(password).digest()[4:12]
    return digest.encode('hex').upper()


class DBUsers:

    def __init__(self, conn_or_fn):
        if type(conn_or_fn) in (str, unicode):
            self._conn = sqlite3.connect(conn_or_fn, check_same_thread=False)
        else:
            self._conn = conn_or_fn
        cur = self._conn.cursor()

        self._conn.text_factory = str

        # MLPasswordHash uses 'hash_minilyrics_client_password'
        sql = '''CREATE TABLE IF NOT EXISTS users (
        id integer primary key AUTOINCREMENT, MLPasswordHash, PasswordHash text,
        Email text, EmailLower text, CreateDate date, LastLoginDate date, 
        LastPasswordChangedDate date, UserName text, UserNameLower text
        );'''
        cur.execute(sql)

        index_fields = ['UserNameLower', 'EmailLower']

        for name in index_fields:
            cur.execute(
                "CREATE UNIQUE INDEX IF NOT EXISTS users_{0} on users ({0});".format(name))

    def close(self):
        self._conn.close()
        self._conn = None

    def import_file(self, fn):
        fields = 'PasswordHash,Email,IsLockedOut,CreateDate,LastLoginDate,LastPasswordChangedDate,LastLockoutDate,UserName,id,MLPasswordHash,UserNameLower,EmailLower'
        fields = fields.split(',')

        removed_fields = 'IsLockedOut,LastLockoutDate'.split(',')
        index_removed_fields = []
        for name in removed_fields:
            index = fields.index(name)
            assert index != -1
            index_removed_fields.append(index)
            fields.remove(name)
        index_removed_fields.reverse()

        index_password = fields.index('PasswordHash')
        assert index_password != -1

        index_Email = fields.index('Email')
        assert index_Email != -1

        index_UserName = fields.index('UserName')
        assert index_UserName != -1

        count_fields = len(fields)

        sql = 'insert into users ({}) values ({})'.format(','.join(fields), ','.join('?' * len(fields)))

        count = 0
        ignored_count = 0

        cur = self._conn.cursor()
        with open(fn, 'rb') as fp:
            fp.readline() # Ignore first line of format
            line = fp.readline()
            while line:
                line = line.strip()
                cols = line.split('\t')
                for index in index_removed_fields:
                    del cols[index]

                password = cols[index_password]
                cols[index_password] = hash_password(password)
                cols.append(hash_minilyrics_client_password(password))
                cols.append(cols[index_UserName].lower())
                cols.append(cols[index_Email].lower())

                if len(cols) == count_fields:
                    cur.execute(sql, cols)
                    count += 1
                    if count % 5000 == 0:
                        print 'Imported {} lines'.format(count)
                else:
                    ignored_count += 1
                    print 'Ignored {} line: {}'.format(ignored_count, line)

                line = fp.readline()

        self._conn.commit()
        
        cur.execute('select count(*) from users')
        row = cur.fetchone()
        print 'Total records', row[0]
        print 'Lines:', count

    def dumpTopN(self, n):
        cur = self._conn.cursor()
        cur.execute('select * from users limit {};'.format(n))

        names = list(map(lambda x: x[0], cur.description))
        print names

        while True:
            row = cur.fetchone()
            if row:
                values = []
                for i, col in enumerate(row):
                    values.append('{}: {}'.format(names[i], col))
                print ', '.join(values)
            else:
                break


def test():
    assert(hex_to_str('0x61') == 'a')

def import_lyrics_db(work_dir):
    db = DBLyrics(os.path.join(work_dir, 'lyrics.db'))
    
    # fn_lyrics = os.path.join(work_dir, 'lyrics.txt')
    fn_lyrics = '/Users/hongyongxiao/Private/MLServer/lyrics.txt'
    db.import_file(fn_lyrics)
    
    db.dumpTopN(100)
    
    db.close()

def import_users_db(work_dir):
    db = DBUsers(os.path.join(work_dir, 'users.db'))
    
    fn_users = '/Users/hongyongxiao/Private/MLServer/users.txt'
    db.import_file(fn_users)
    
    db.dumpTopN(100)
    
    db.close()

def main():
    test()
    
    work_dir = os.path.abspath(os.path.dirname(__file__))
    import_lyrics_db(work_dir)
    import_users_db(work_dir)


if __name__ == '__main__':
    main()
