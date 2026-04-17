import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.base import MIMEBase
from email import encoders
import json
from datetime import datetime

_handler_registry = {}
_handler_id_counter = 1

class SmtpHandler:
    def __init__(self):
        self.smtp = None
        self.username = ""
        self.password = ""
        self.server = ""
        self.port = 587
        self.is_connected = False
        self.use_tls = True
        self.last_error = ""

    def connect(self, server, port, username, password, use_tls=True):
        try:
            self.server = server
            self.port = port
            self.username = username
            self.password = password
            self.use_tls = use_tls
            
            self.smtp = smtplib.SMTP(server, port, timeout=30)
            self.smtp.ehlo()
            
            if use_tls:
                self.smtp.starttls()
                self.smtp.ehlo()
            
            self.smtp.login(username, password)
            self.is_connected = True
            return True
        except smtplib.SMTPAuthenticationError as e:
            self.last_error = "认证失败! 请检查用户名和密码是否正确"
            self.is_connected = False
            return False
        except smtplib.SMTPException as e:
            self.last_error = f"SMTP错误: {str(e)}"
            self.is_connected = False
            return False
        except Exception as e:
            self.last_error = f"连接错误: {str(e)}"
            self.is_connected = False
            return False

    def disconnect(self):
        try:
            if self.smtp:
                try:
                    self.smtp.quit()
                except:
                    pass
            self.smtp = None
            self.is_connected = False
            return True
        except Exception as e:
            self.last_error = f"断开连接错误: {str(e)}"
            return False

    def send_email(self, from_addr, from_name, to_addrs, subject, body, html_body=None, cc_addrs=None, bcc_addrs=None, attachments=None):
        try:
            if not self.is_connected or not self.smtp:
                self.last_error = "未连接到SMTP服务器"
                return False
            
            msg = MIMEMultipart('alternative')
            msg['From'] = f"{from_name} <{from_addr}>" if from_name else from_addr
            msg['To'] = ', '.join(to_addrs) if isinstance(to_addrs, list) else to_addrs
            msg['Subject'] = subject
            msg['Date'] = datetime.now().strftime('%a, %d %b %Y %H:%M:%S +0000')
            msg['Message-ID'] = f"<{datetime.now().strftime('%Y%m%d%H%M%S')}@dataassistant>"
            
            if cc_addrs:
                if isinstance(cc_addrs, str):
                    msg['Cc'] = cc_addrs
                else:
                    msg['Cc'] = ', '.join(cc_addrs)
            
            if html_body:
                part1 = MIMEText(body if body else '', 'plain', 'utf-8')
                part2 = MIMEText(html_body, 'html', 'utf-8')
                msg.attach(part1)
                msg.attach(part2)
            else:
                part1 = MIMEText(body if body else '', 'plain', 'utf-8')
                msg.attach(part1)
            
            if attachments:
                for attachment in attachments:
                    if isinstance(attachment, dict):
                        filename = attachment.get('filename', 'attachment')
                        content = attachment.get('content', b'')
                        content_type = attachment.get('content_type', 'application/octet-stream')
                    elif isinstance(attachment, tuple) and len(attachment) >= 2:
                        filename, content = attachment[0], attachment[1]
                        content_type = attachment[2] if len(attachment) > 2 else 'application/octet-stream'
                    else:
                        continue
                    
                    part = MIMEBase('application', 'octet-stream')
                    part.set_payload(content)
                    encoders.encode_base64(part)
                    part.add_header('Content-Disposition', f'attachment; filename="{filename}"')
                    part.add_header('Content-Type', content_type)
                    msg.attach(part)
            
            all_recipients = []
            if isinstance(to_addrs, list):
                all_recipients.extend(to_addrs)
            else:
                all_recipients.append(to_addrs)
            
            if cc_addrs:
                if isinstance(cc_addrs, str):
                    all_recipients.extend([addr.strip() for addr in cc_addrs.split(',')])
                else:
                    all_recipients.extend(cc_addrs)
            
            if bcc_addrs:
                if isinstance(bcc_addrs, str):
                    all_recipients.extend([addr.strip() for addr in bcc_addrs.split(',')])
                else:
                    all_recipients.extend(bcc_addrs)
            
            self.smtp.sendmail(from_addr, all_recipients, msg.as_string())
            return True
        except smtplib.SMTPRecipientsRefused as e:
            self.last_error = f"收件人被拒绝: {str(e)}"
            return False
        except smtplib.SMTPSenderRefused as e:
            self.last_error = f"发件人被拒绝: {str(e)}"
            return False
        except smtplib.SMTPServerDisconnected as e:
            self.last_error = "服务器断开连接"
            self.is_connected = False
            return False
        except Exception as e:
            self.last_error = f"发送邮件错误: {str(e)}"
            return False

    def send_simple_email(self, to_addr, subject, body):
        try:
            if not self.is_connected or not self.smtp:
                self.last_error = "未连接到SMTP服务器"
                return False
            
            msg = MIMEText(body, 'plain', 'utf-8')
            msg['From'] = self.username
            msg['To'] = to_addr
            msg['Subject'] = subject
            
            self.smtp.send_message(msg)
            return True
        except Exception as e:
            self.last_error = f"发送简单邮件错误: {str(e)}"
            return False

    def verify_connection(self):
        try:
            if self.smtp:
                status = self.smtp.noop()
                return status[0] == 250
            return False
        except:
            return False

    def get_last_error(self):
        return self.last_error

    def is_connected_to_server(self):
        return self.is_connected

def create_handler():
    global _handler_registry, _handler_id_counter
    handler = SmtpHandler()
    handler_id = _handler_id_counter
    _handler_registry[handler_id] = handler
    _handler_id_counter += 1
    return handler_id

def _get_handler(handler_id):
    return _handler_registry.get(handler_id)

def handler_connect(handler_id, server, port, username, password, use_tls=True):
    handler = _get_handler(handler_id)
    if not handler:
        return False, "Handler not found"
    return handler.connect(server, port, username, password, use_tls)

def handler_disconnect(handler_id):
    handler = _get_handler(handler_id)
    if not handler:
        return False, "Handler not found"
    return handler.disconnect()

def handler_send_email(handler_id, from_addr, from_name, to_addrs, subject, body, html_body=None, cc_addrs=None, bcc_addrs=None, attachments=None):
    handler = _get_handler(handler_id)
    if not handler:
        return False, "Handler not found"
    return handler.send_email(from_addr, from_name, to_addrs, subject, body, html_body, cc_addrs, bcc_addrs, attachments)

def handler_send_simple(handler_id, to_addr, subject, body):
    handler = _get_handler(handler_id)
    if not handler:
        return False, "Handler not found"
    return handler.send_simple_email(to_addr, subject, body)

def handler_verify(handler_id):
    handler = _get_handler(handler_id)
    if not handler:
        return False, "Handler not found"
    return handler.verify_connection()

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