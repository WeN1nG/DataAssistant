import imaplib
import email
from email.header import decode_header
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.utils import parsedate_to_datetime
import json
import base64
from datetime import datetime

_handler_registry = {}
_handler_id_counter = 1

class ImapHandler:
    def __init__(self):
        self.mail = None
        self.username = ""
        self.password = ""
        self.server = ""
        self.port = 993
        self.is_connected = False
        self.current_mailbox = "INBOX"
        self.last_error = ""

    def connect(self, server, port, username, password):
        print(f"[ImapHandler.connect] Called with:")
        print(f"  server: '{server}' (type: {type(server)})")
        print(f"  port: {port} (type: {type(port)})")
        print(f"  username: '{username}' (type: {type(username)})")
        print(f"  password: '****' (length: {len(password) if password else 0})")
        
        try:
            self.server = server
            self.port = port
            self.username = username
            self.password = password
            
            print(f"[ImapHandler.connect] Creating IMAP4_SSL connection to {server}:{port}")
            self.mail = imaplib.IMAP4_SSL(server, port)
            self.mail.socket().settimeout(30)
            print(f"[ImapHandler.connect] Set socket timeout to 30 seconds")
            print(f"[ImapHandler.connect] Calling mail.login('{username}', ****)")
            self.mail.login(username, password)
            self.is_connected = True
            print(f"[ImapHandler.connect] Login successful!")
            return True
        except imaplib.IMAP4.error as e:
            error_msg = str(e)
            if 'AUTHENTICATIONFAILED' in error_msg.upper() or 'authentication' in error_msg.lower():
                self.last_error = "认证失败! 请检查用户名和密码是否正确"
            elif 'LOGIN' in error_msg.upper() and 'BAD' in error_msg.upper():
                self.last_error = "登录参数错误! 请检查用户名和密码格式"
            else:
                self.last_error = f"IMAP服务器错误: {error_msg}"
            self.is_connected = False
            return False
        except Exception as e:
            self.last_error = f"连接错误: {str(e)}"
            self.is_connected = False
            return False

    def disconnect(self):
        try:
            if self.mail:
                try:
                    self.mail.logout()
                except:
                    self.mail.close()
            self.mail = None
            self.is_connected = False
            return True
        except Exception as e:
            self.last_error = f"断开连接错误: {str(e)}"
            return False

    def select_mailbox(self, mailbox_name):
        print(f"[DEBUG] ImapHandler.select_mailbox called:")
        print(f"  mailbox_name: '{mailbox_name}'")
        print(f"  is_connected: {self.is_connected}")
        print(f"  self.mail: {self.mail}")
        
        try:
            print(f"[DEBUG] ImapHandler.select_mailbox: Calling self.mail.select({mailbox_name})...")
            status, messages = self.mail.select(mailbox_name)
            print(f"[DEBUG] ImapHandler.select_mailbox: self.mail.select returned, status: {status}, messages: {messages}")
            
            if status == 'OK':
                self.current_mailbox = mailbox_name
                print(f"[DEBUG] ImapHandler.select_mailbox: SUCCESS, current_mailbox set to: {self.current_mailbox}")
                return True
            else:
                self.last_error = f"选择邮箱失败: {status}"
                print(f"[ERROR] ImapHandler.select_mailbox: FAILED, status: {status}")
                print(f"[ERROR] ImapHandler.select_mailbox: last_error: {self.last_error}")
                return False
        except Exception as e:
            self.last_error = f"选择邮箱错误: {str(e)}"
            print(f"[ERROR] ImapHandler.select_mailbox: EXCEPTION: {str(e)}")
            import traceback
            traceback.print_exc()
            return False

    def list_mailboxes(self):
        try:
            status, mailboxes = self.mail.list()
            if status == 'OK':
                result = []
                for mailbox in mailboxes:
                    parts = mailbox.decode('utf-8', errors='replace').split('"')
                    if len(parts) >= 2:
                        result.append(parts[-2].strip())
                    else:
                        result.append(mailbox.decode('utf-8', errors='replace').strip())
                return result
            else:
                self.last_error = f"获取邮箱列表失败: {status}"
                return []
        except Exception as e:
            self.last_error = f"获取邮箱列表错误: {str(e)}"
            return []

    def get_mailbox_status(self, mailbox_name="INBOX"):
        try:
            status, data = self.mail.status(mailbox_name, "(MESSAGES RECENT UNSEEN)")
            if status == 'OK':
                result = {'messages': 0, 'recent': 0, 'unseen': 0}
                for item in data[0].decode('utf-8', errors='replace').split():
                    if '(' in item:
                        parts = item.split('(')
                        if len(parts) >= 2:
                            key = parts[0].strip()
                            try:
                                value = int(parts[1].strip(')'))
                                if key == 'MESSAGES':
                                    result['messages'] = value
                                elif key == 'RECENT':
                                    result['recent'] = value
                                elif key == 'UNSEEN':
                                    result['unseen'] = value
                            except:
                                pass
                return result
            else:
                self.last_error = f"获取邮箱状态失败: {status}"
                return {'messages': 0, 'recent': 0, 'unseen': 0}
        except Exception as e:
            self.last_error = f"获取邮箱状态错误: {str(e)}"
            return {'messages': 0, 'recent': 0, 'unseen': 0}

    def search_emails(self, criteria=None):
        try:
            if criteria is None:
                status, message_ids = self.mail.search(None, 'ALL')
            else:
                status, message_ids = self.mail.search(None, criteria)
            
            if status == 'OK':
                return message_ids[0].split()
            else:
                self.last_error = f"搜索邮件失败: {status}"
                return []
        except Exception as e:
            self.last_error = f"搜索邮件错误: {str(e)}"
            return []

    def fetch_email_headers(self, email_id):
        import sys
        try:
            status, msg_data = self.mail.fetch(email_id, '(RFC822.HEADER)')
            if status == 'OK' and msg_data and msg_data[0]:
                email_info = {
                    'id': email_id.decode('utf-8') if isinstance(email_id, bytes) else str(email_id),
                    'subject': '',
                    'from': '',
                    'to': '',
                    'date': '',
                    'flags': ''
                }
                
                raw_data = msg_data[0][1] if isinstance(msg_data[0], tuple) else msg_data[0]
                
                try:
                    msg = email.message_from_bytes(raw_data)
                    
                    raw_subject = msg.get('Subject')
                    raw_from = msg.get('From')
                    raw_to = msg.get('To')
                    raw_date = msg.get('Date')
                    
                    sys.stderr.write(f"[DEBUG] fetch_email_headers: raw_subject type={type(raw_subject)}, value='{str(raw_subject)[:50] if raw_subject else 'None'}'\n")
                    sys.stderr.flush()
                    
                    if raw_subject is None:
                        raw_subject = ''
                    if raw_from is None:
                        raw_from = ''
                    if raw_to is None:
                        raw_to = ''
                    if raw_date is None:
                        raw_date = ''
                    
                    email_info['subject'] = self.decode_header_value(raw_subject)
                    email_info['from'] = self.decode_header_value(raw_from)
                    email_info['to'] = self.decode_header_value(raw_to)
                    email_info['date'] = self.parse_email_date(raw_date)
                    
                    sys.stderr.write(f"[DEBUG] fetch_email_headers: decoded subject='{email_info['subject'][:50]}', from='{email_info['from'][:50]}'\n")
                    sys.stderr.flush()
                except Exception as parse_error:
                    sys.stderr.write(f"[ERROR] fetch_email_headers: parse error: {str(parse_error)}\n")
                    sys.stderr.flush()
                    import traceback
                    traceback.print_exc()
                
                return email_info
            else:
                sys.stderr.write(f"[ERROR] fetch_email_headers: status={status}, msg_data={msg_data}\n")
                sys.stderr.flush()
            return None
        except Exception as e:
            self.last_error = f"获取邮件头错误: {str(e)}"
            sys.stderr.write(f"[ERROR] fetch_email_headers exception: {str(e)}\n")
            sys.stderr.flush()
            return None

    def fetch_email_body(self, email_id):
        import sys
        try:
            sys.stderr.write(f"[DEBUG] fetch_email_body: fetching email_id={email_id}\n")
            sys.stderr.flush()
            
            print(f"[DEBUG] ImapHandler.fetch_email_body: email_id={email_id}, type={type(email_id)}")
            print(f"[DEBUG] ImapHandler.fetch_email_body: is_connected={self.is_connected}")
            print(f"[DEBUG] ImapHandler.fetch_email_body: current_mailbox={self.current_mailbox}")
            
            status, msg_data = self.mail.fetch(email_id, '(RFC822)')
            sys.stderr.write(f"[DEBUG] fetch_email_body: fetch completed, status={status}\n")
            sys.stderr.flush()
            
            print(f"[DEBUG] ImapHandler.fetch_email_body: mail.fetch returned, status={status}")
            print(f"[DEBUG] ImapHandler.fetch_email_body: msg_data type={type(msg_data)}, length={len(msg_data) if msg_data else 0}")
            
            if status == 'OK' and msg_data and msg_data[0]:
                raw_email = msg_data[0][1] if isinstance(msg_data[0], tuple) else msg_data[0]
                msg = email.message_from_bytes(raw_email)
                
                print(f"[DEBUG] ImapHandler.fetch_email_body: Parsing email message...")
                
                result = {
                    'id': email_id.decode('utf-8') if isinstance(email_id, bytes) else str(email_id),
                    'subject': self.decode_header_value(msg.get('Subject', '')),
                    'from': self.decode_header_value(msg.get('From', '')),
                    'to': self.decode_header_value(msg.get('To', '')),
                    'cc': self.decode_header_value(msg.get('Cc', '')),
                    'date': self.parse_email_date(msg.get('Date', '')),
                    'body': '',
                    'html_body': '',
                    'attachments': []
                }
                
                print(f"[DEBUG] ImapHandler.fetch_email_body: msg.is_multipart={msg.is_multipart()}")
                
                if msg.is_multipart():
                    print(f"[DEBUG] ImapHandler.fetch_email_body: Processing multipart message...")
                    for part in msg.walk():
                        content_type = part.get_content_type()
                        content_disposition = str(part.get('Content-Disposition', ''))
                        
                        if content_disposition and 'attachment' in content_disposition:
                            filename = part.get_filename()
                            if filename:
                                decoded_filename = self.decode_header_value(filename)
                                try:
                                    attachment_content = part.get_payload(decode=True)
                                    if attachment_content:
                                        content_base64 = base64.b64encode(attachment_content).decode('utf-8')
                                        result['attachments'].append({
                                            'filename': decoded_filename,
                                            'content_type': content_type,
                                            'content': content_base64,
                                            'size': len(attachment_content)
                                        })
                                        print(f"[DEBUG] ImapHandler.fetch_email_body: Extracted attachment: {decoded_filename}, size={len(attachment_content)}")
                                    else:
                                        result['attachments'].append({
                                            'filename': decoded_filename,
                                            'content_type': content_type,
                                            'content': '',
                                            'size': 0
                                        })
                                except Exception as e:
                                    print(f"[ImapHandler] Error extracting attachment: {e}")
                                    result['attachments'].append({
                                        'filename': decoded_filename,
                                        'content_type': content_type,
                                        'content': '',
                                        'size': 0
                                    })
                        
                        if content_type == 'text/plain' and 'attachment' not in content_disposition:
                            if not result['body']:
                                try:
                                    charset = part.get_content_charset() or 'utf-8'
                                    decoded_payload = part.get_payload(decode=True)
                                    if decoded_payload:
                                        body_text = decoded_payload.decode(charset, errors='replace')
                                        if len(body_text) > 100000:
                                            body_text = body_text[:100000] + "\n[内容已截断，超出大小限制]"
                                        result['body'] = self.clean_text(body_text)
                                        print(f"[DEBUG] ImapHandler.fetch_email_body: Extracted text body, length={len(result['body'])}")
                                except:
                                    pass
                        
                        elif content_type == 'text/html' and 'attachment' not in content_disposition:
                            if not result['html_body']:
                                try:
                                    charset = part.get_content_charset() or 'utf-8'
                                    decoded_payload = part.get_payload(decode=True)
                                    if decoded_payload:
                                        html_text = decoded_payload.decode(charset, errors='replace')
                                        if len(html_text) > 200000:
                                            html_text = html_text[:200000] + "\n<!-- 内容已截断，超出大小限制 -->"
                                        result['html_body'] = self.clean_html(html_text)
                                        print(f"[DEBUG] ImapHandler.fetch_email_body: Extracted HTML body, length={len(result['html_body'])}")
                                except:
                                    pass
                else:
                    print(f"[DEBUG] ImapHandler.fetch_email_body: Processing non-multipart message...")
                    try:
                        charset = msg.get_content_charset() or 'utf-8'
                        decoded_payload = msg.get_payload(decode=True)
                        if decoded_payload:
                            content = decoded_payload.decode(charset, errors='replace')
                        else:
                            content = str(msg.get_payload())
                        
                        if msg.get_content_type() == 'text/html':
                            if len(content) > 200000:
                                content = content[:200000] + "\n<!-- 内容已截断，超出大小限制 -->"
                            result['html_body'] = self.clean_html(content)
                            print(f"[DEBUG] ImapHandler.fetch_email_body: Extracted HTML body (non-multipart), length={len(result['html_body'])}")
                        else:
                            if len(content) > 100000:
                                content = content[:100000] + "\n[内容已截断，超出大小限制]"
                            result['body'] = self.clean_text(content)
                            print(f"[DEBUG] ImapHandler.fetch_email_body: Extracted text body (non-multipart), length={len(result['body'])}")
                    except:
                        result['body'] = str(msg.get_payload())
                
                print(f"[DEBUG] ImapHandler.fetch_email_body: Returning result with body.length={len(result['body'])}, html_body.length={len(result['html_body'])}, attachments={len(result['attachments'])}")
                return result
            else:
                print(f"[ERROR] ImapHandler.fetch_email_body: status={status}, msg_data={msg_data}")
                return None
        except Exception as e:
            self.last_error = f"获取邮件内容错误: {str(e)}"
            print(f"[ERROR] ImapHandler.fetch_email_body: EXCEPTION: {str(e)}")
            import traceback
            traceback.print_exc()
            return None

    def delete_email(self, email_id):
        try:
            self.mail.store(email_id, '+FLAGS', '\\Deleted')
            self.mail.expunge()
            return True
        except Exception as e:
            self.last_error = f"删除邮件错误: {str(e)}"
            return False

    def move_email(self, email_id, destination_mailbox):
        try:
            self.mail.copy(email_id, destination_mailbox)
            self.mail.store(email_id, '+FLAGS', '\\Deleted')
            self.mail.expunge()
            return True
        except Exception as e:
            self.last_error = f"移动邮件错误: {str(e)}"
            return False

    def set_flags(self, email_id, flags):
        try:
            self.mail.store(email_id, '+FLAGS', flags)
            return True
        except Exception as e:
            self.last_error = f"设置标志错误: {str(e)}"
            return False

    def get_recent_emails(self, count=10):
        import sys
        try:
            sys.stderr.write("[DEBUG] get_recent_emails: Starting\n")
            sys.stderr.flush()
            
            status, message_ids = self.mail.search(None, 'ALL')
            sys.stderr.write(f"[DEBUG] get_recent_emails: search status={status}\n")
            sys.stderr.flush()
            
            if status != 'OK':
                self.last_error = f"获取邮件列表失败: {status}"
                sys.stderr.write(f"[ERROR] get_recent_emails: search failed\n")
                sys.stderr.flush()
                return []
            
            if not message_ids or not message_ids[0]:
                sys.stderr.write("[DEBUG] get_recent_emails: no message IDs\n")
                sys.stderr.flush()
                return []
            
            email_ids = message_ids[0].split()
            sys.stderr.write(f"[DEBUG] get_recent_emails: found {len(email_ids)} emails\n")
            sys.stderr.flush()
            
            if not email_ids:
                sys.stderr.write("[DEBUG] get_recent_emails: email_ids empty after split\n")
                sys.stderr.flush()
                return []
            
            start_index = max(0, len(email_ids) - count)
            recent_ids = email_ids[start_index:]
            sys.stderr.write(f"[DEBUG] get_recent_emails: fetching last {count} from index {start_index}\n")
            sys.stderr.flush()
            
            results = []
            for i, email_id in enumerate(recent_ids):
                try:
                    sys.stderr.write(f"[DEBUG] get_recent_emails: fetching email {i+1}/{len(recent_ids)}\n")
                    sys.stderr.flush()
                    
                    email_data = self.fetch_email_body(email_id)
                    sys.stderr.write(f"[DEBUG] get_recent_emails: fetch returned type={type(email_data)}\n")
                    sys.stderr.flush()
                    
                    if email_data:
                        safe_subject = str(email_data.get('subject', 'N/A'))[:50]
                        sys.stderr.write(f"[DEBUG] get_recent_emails: success, subject={safe_subject}\n")
                        sys.stderr.flush()
                        results.append(email_data)
                    else:
                        sys.stderr.write(f"[DEBUG] get_recent_emails: fetch returned None\n")
                        sys.stderr.flush()
                except Exception as fetch_error:
                    sys.stderr.write(f"[ERROR] get_recent_emails: {str(fetch_error)}\n")
                    sys.stderr.flush()
            
            sys.stderr.write(f"[DEBUG] get_recent_emails: total results={len(results)}\n")
            sys.stderr.flush()
            
            total_body_size = sum(len(r.get('body', '')) for r in results)
            total_html_size = sum(len(r.get('html_body', '')) for r in results)
            sys.stderr.write(f"[DEBUG] get_recent_emails: total_body_size={total_body_size}, total_html_size={total_html_size}\n")
            sys.stderr.flush()
            
            sys.stderr.write("[DEBUG] get_recent_emails: returning results now...\n")
            sys.stderr.flush()
            return results
        except Exception as e:
            self.last_error = f"获取最近邮件错误: {str(e)}"
            sys.stderr.write(f"[ERROR] get_recent_emails exception: {str(e)}\n")
            sys.stderr.flush()
            import traceback
            traceback.print_exc()
            return []

    def get_recent_emails_fast(self, count=50):
        import sys
        try:
            sys.stderr.write("[DEBUG] get_recent_emails_fast: Starting (headers only)\n")
            sys.stderr.flush()
            
            status, message_ids = self.mail.search(None, 'ALL')
            sys.stderr.write(f"[DEBUG] get_recent_emails_fast: search status={status}\n")
            sys.stderr.flush()
            
            if status != 'OK':
                self.last_error = f"获取邮件列表失败: {status}"
                sys.stderr.write(f"[ERROR] get_recent_emails_fast: search failed\n")
                sys.stderr.flush()
                return []
            
            if not message_ids or not message_ids[0]:
                sys.stderr.write("[DEBUG] get_recent_emails_fast: no message IDs\n")
                sys.stderr.flush()
                return []
            
            email_ids = message_ids[0].split()
            sys.stderr.write(f"[DEBUG] get_recent_emails_fast: found {len(email_ids)} emails\n")
            sys.stderr.flush()
            
            start_index = max(0, len(email_ids) - count)
            recent_ids = email_ids[start_index:]
            sys.stderr.write(f"[DEBUG] get_recent_emails_fast: fetching last {count} from index {start_index}\n")
            sys.stderr.flush()
            
            results = []
            for i, email_id in enumerate(recent_ids):
                try:
                    email_data = self.fetch_email_headers(email_id)
                    if email_data:
                        results.append(email_data)
                        if i < 3:
                            sys.stderr.write(f"[DEBUG] get_recent_emails_fast: added email {i+1}, subject={email_data.get('subject', 'N/A')[:30]}\n")
                            sys.stderr.flush()
                    else:
                        sys.stderr.write(f"[DEBUG] get_recent_emails_fast: email_data is None for {email_id}\n")
                        sys.stderr.flush()
                    if (i + 1) % 10 == 0:
                        sys.stderr.write(f"[DEBUG] get_recent_emails_fast: fetched {i+1}/{len(recent_ids)}\n")
                        sys.stderr.flush()
                except Exception as fetch_error:
                    sys.stderr.write(f"[ERROR] get_recent_emails_fast: {str(fetch_error)}\n")
                    sys.stderr.flush()
            
            sys.stderr.write(f"[DEBUG] get_recent_emails_fast: total results={len(results)}\n")
            sys.stderr.flush()
            return results
        except Exception as e:
            self.last_error = f"获取最近邮件错误: {str(e)}"
            sys.stderr.write(f"[ERROR] get_recent_emails_fast exception: {str(e)}\n")
            sys.stderr.flush()
            import traceback
            traceback.print_exc()
            return []

    def decode_header_value(self, header_str):
        import sys
        if header_str is None:
            return ''
        if header_str == '':
            return ''
        
        try:
            decoded_parts = decode_header(header_str)
            
            if not decoded_parts:
                return ''
            
            result = []
            
            for part, encoding in decoded_parts:
                if isinstance(part, bytes):
                    try:
                        if encoding:
                            result.append(part.decode(encoding))
                        else:
                            result.append(part.decode('utf-8', errors='replace'))
                    except:
                        try:
                            result.append(part.decode('gb2312', errors='replace'))
                        except:
                            result.append(part.decode('utf-8', errors='replace'))
                elif isinstance(part, str):
                    result.append(part)
                elif part is not None:
                    result.append(str(part))
            
            decoded = ''.join(result)
            return decoded
        except Exception as e:
            sys.stderr.write(f"[ERROR] decode_header_value failed: {str(e)}, header_str='{str(header_str)[:50]}'\n")
            sys.stderr.flush()
            return str(header_str) if header_str else ''
    
    def parse_email_date(self, date_header):
        import sys
        if not date_header or date_header == '':
            return ''
        
        try:
            dt = parsedate_to_datetime(date_header)
            return dt.strftime('%Y-%m-%dT%H:%M:%S')
        except Exception as e:
            sys.stderr.write(f"[WARN] parse_email_date failed to parse: '{date_header[:50]}', error: {str(e)}\n")
            sys.stderr.flush()
            return ''
    
    def clean_html(self, html_content):
        if not html_content:
            return ''
        
        try:
            content = html_content.strip()
            
            if '<html' not in content.lower():
                content = f'<!DOCTYPE html><html><head><meta charset="utf-8"></head><body>{content}</body></html>'
            elif '<meta' in content.lower() and 'charset' not in content.lower():
                content = content.replace('<head>', '<head><meta charset="utf-8">', 1)
            
            return content
        except:
            return html_content
    
    def clean_text(self, text_content):
        if not text_content:
            return ''
        
        try:
            text = text_content.strip()
            
            text = text.replace('\r\n', '\n')
            text = text.replace('\r', '\n')
            
            while '\n\n\n' in text:
                text = text.replace('\n\n\n', '\n\n')
            
            return text
        except:
            return text_content

    def get_last_error(self):
        return self.last_error

    def is_connected_to_server(self):
        return self.is_connected

def create_handler():
    global _handler_registry, _handler_id_counter
    handler = ImapHandler()
    handler_id = _handler_id_counter
    _handler_registry[handler_id] = handler
    _handler_id_counter += 1
    return handler_id

def _get_handler(handler_id):
    return _handler_registry.get(handler_id)

def handler_connect(handler_id, server, port, username, password):
    print(f"[DEBUG] handler_connect called:")
    print(f"  handler_id: {handler_id} (type: {type(handler_id)})")
    print(f"  server: {server} (type: {type(server)})")
    print(f"  port: {port} (type: {type(port)})")
    print(f"  username: {username} (type: {type(username)})")
    print(f"  password: {password} (type: {type(password)})")
    print(f"  password length: {len(password) if password else 0}")
    
    handler = _get_handler(handler_id)
    if not handler:
        print(f"[DEBUG] Handler not found for id: {handler_id}")
        return False, "Handler not found"
    
    print(f"[DEBUG] Calling handler.connect({server}, {port}, {username}, ****)")
    success = handler.connect(server, port, username, password)
    error_msg = handler.get_last_error() if not success else ""
    print(f"[DEBUG] connect result: {success}, error: {error_msg}")
    return success, error_msg

def handler_disconnect(handler_id):
    handler = _get_handler(handler_id)
    if not handler:
        return False, "Handler not found"
    return handler.disconnect()

def handler_select_mailbox(handler_id, mailbox_name):
    print(f"[DEBUG] handler_select_mailbox called:")
    print(f"  handler_id: {handler_id} (type: {type(handler_id)})")
    print(f"  mailbox_name: '{mailbox_name}' (type: {type(mailbox_name)})")
    
    handler = _get_handler(handler_id)
    if not handler:
        print(f"[DEBUG] handler_select_mailbox: Handler not found for id: {handler_id}")
        return False, "Handler not found"
    
    print(f"[DEBUG] handler_select_mailbox: Handler found, calling select_mailbox...")
    print(f"[DEBUG] handler_select_mailbox: handler.is_connected: {handler.is_connected}")
    print(f"[DEBUG] handler_select_mailbox: handler.username: {handler.username}")
    
    result = handler.select_mailbox(mailbox_name)
    error_msg = handler.get_last_error() if not result else ""
    print(f"[DEBUG] handler_select_mailbox: select_mailbox result: {result}, error: {error_msg}")
    return result, error_msg

def handler_list_mailboxes(handler_id):
    handler = _get_handler(handler_id)
    if not handler:
        return [], "Handler not found"
    return handler.list_mailboxes()

def handler_get_status(handler_id, mailbox_name="INBOX"):
    handler = _get_handler(handler_id)
    if not handler:
        return None, "Handler not found"
    return handler.get_mailbox_status(mailbox_name)

def handler_search(handler_id, criteria=None):
    handler = _get_handler(handler_id)
    if not handler:
        return [], "Handler not found"
    return handler.search_emails(criteria)

def handler_fetch_headers(handler_id, email_id):
    handler = _get_handler(handler_id)
    if not handler:
        return None, "Handler not found"
    return handler.fetch_email_headers(email_id)

def handler_fetch_body(handler_id, email_id):
    print(f"[DEBUG] handler_fetch_body called:")
    print(f"  handler_id: {handler_id} (type: {type(handler_id)})")
    print(f"  email_id: {email_id} (type: {type(email_id)})")
    
    handler = _get_handler(handler_id)
    if not handler:
        print(f"[DEBUG] handler_fetch_body: Handler not found for id: {handler_id}")
        return None
    
    print(f"[DEBUG] handler_fetch_body: Handler found, calling fetch_email_body...")
    print(f"[DEBUG] handler_fetch_body: handler.is_connected: {handler.is_connected}")
    print(f"[DEBUG] handler_fetch_body: handler.current_mailbox: {handler.current_mailbox}")
    
    result = handler.fetch_email_body(email_id)
    
    if result:
        print(f"[DEBUG] handler_fetch_body: fetch_email_body returned, subject: '{result.get('subject', '')[:50]}'")
        print(f"[DEBUG] handler_fetch_body: body length: {len(result.get('body', ''))}, html_body length: {len(result.get('html_body', ''))}")
    else:
        print(f"[DEBUG] handler_fetch_body: fetch_email_body returned None or empty")
    
    return result

def handler_delete(handler_id, email_id):
    handler = _get_handler(handler_id)
    if not handler:
        return False, "Handler not found"
    return handler.delete_email(email_id)

def handler_move(handler_id, email_id, destination):
    handler = _get_handler(handler_id)
    if not handler:
        return False, "Handler not found"
    return handler.move_email(email_id, destination)

def handler_set_flags(handler_id, email_id, flags):
    handler = _get_handler(handler_id)
    if not handler:
        return False, "Handler not found"
    return handler.set_flags(email_id, flags)

def handler_get_recent(handler_id, count=10):
    handler = _get_handler(handler_id)
    if not handler:
        return [], "Handler not found"
    
    try:
        result = handler.get_recent_emails(count)
        return result
    except Exception as e:
        import sys
        sys.stderr.write(f"[ERROR] handler_get_recent exception: {str(e)}\n")
        sys.stderr.flush()
        return []

def handler_get_recent_fast(handler_id, count=50):
    import sys
    handler = _get_handler(handler_id)
    if not handler:
        sys.stderr.write("[DEBUG] handler_get_recent_fast: handler not found\n")
        sys.stderr.flush()
        return []
    
    try:
        sys.stderr.write(f"[DEBUG] handler_get_recent_fast: calling get_recent_emails_fast with count={count}\n")
        sys.stderr.flush()
        result = handler.get_recent_emails_fast(count)
        sys.stderr.write(f"[DEBUG] handler_get_recent_fast: got {len(result)} results\n")
        sys.stderr.flush()
        return result
    except Exception as e:
        sys.stderr.write(f"[ERROR] handler_get_recent_fast exception: {str(e)}\n")
        sys.stderr.flush()
        import traceback
        traceback.print_exc()
        return []

def handler_get_recent_emails_json(handler_id, count=10):
    import json
    import sys
    handler = _get_handler(handler_id)
    if not handler:
        sys.stderr.write("[DEBUG] handler_get_recent_emails_json: handler not found\n")
        sys.stderr.flush()
        return "{}"
    
    try:
        sys.stderr.write("[DEBUG] handler_get_recent_emails_json: getting recent emails...\n")
        sys.stderr.flush()
        
        result = handler.get_recent_emails(count)
        
        sys.stderr.write(f"[DEBUG] handler_get_recent_emails_json: got {len(result)} emails, starting JSON serialization\n")
        sys.stderr.flush()
        
        json_str = json.dumps(result, ensure_ascii=False)
        
        sys.stderr.write(f"[DEBUG] handler_get_recent_emails_json: JSON serialization complete, length={len(json_str)}\n")
        sys.stderr.flush()
        
        return json_str
    except Exception as e:
        sys.stderr.write(f"[ERROR] handler_get_recent_emails_json exception: {str(e)}\n")
        sys.stderr.flush()
        import traceback
        traceback.print_exc()
        return "{}"

def handler_get_error(handler_id):
    handler = _get_handler(handler_id)
    if not handler:
        return "Handler not found"
    return handler.get_last_error()

def handler_is_connected(handler_id):
    handler = _get_handler(handler_id)
    if not handler:
        return False
    return handler.is_connected_to_server()

def handler_fetch_attachment(handler_id, email_id, filename):
    handler = _get_handler(handler_id)
    if not handler:
        return None
    
    try:
        status, msg_data = handler.mail.fetch(email_id, '(RFC822)')
        if status == 'OK' and msg_data and msg_data[0]:
            raw_email = msg_data[0][1] if isinstance(msg_data[0], tuple) else msg_data[0]
            msg = email.message_from_bytes(raw_email)
            
            if msg.is_multipart():
                for part in msg.walk():
                    content_disposition = str(part.get('Content-Disposition', ''))
                    if content_disposition and 'attachment' in content_disposition:
                        part_filename = part.get_filename()
                        if part_filename:
                            decoded_filename = handler.decode_header_value(part_filename)
                            if decoded_filename == filename:
                                attachment_content = part.get_payload(decode=True)
                                if attachment_content:
                                    content_base64 = base64.b64encode(attachment_content).decode('utf-8')
                                    return {
                                        'filename': decoded_filename,
                                        'content_type': part.get_content_type(),
                                        'content': content_base64,
                                        'size': len(attachment_content)
                                    }
                                else:
                                    return {
                                        'filename': decoded_filename,
                                        'content_type': part.get_content_type(),
                                        'content': '',
                                        'size': 0
                                    }
            return None
        return None
    except Exception as e:
        print(f"[ImapHandler] Error fetching attachment: {e}")
        return None