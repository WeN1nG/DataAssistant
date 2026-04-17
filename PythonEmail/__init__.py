from .imap_handler import ImapHandler, create_handler as create_imap_handler
from .smtp_handler import SmtpHandler, create_handler as create_smtp_handler

__all__ = ['ImapHandler', 'SmtpHandler', 'create_imap_handler', 'create_smtp_handler']

def get_imap_handler():
    return create_imap_handler()

def get_smtp_handler():
    return create_smtp_handler()