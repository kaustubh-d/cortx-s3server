import http.client, urllib.parse
import sys
from s3iamcli.util import sign_request_v2
from s3iamcli.conn_manager import ConnMan
from s3iamcli.error_response import ErrorResponse
from s3iamcli.create_account_response import CreateAccountResponse
from s3iamcli.list_account_response import ListAccountResponse
from s3iamcli.reset_key_response import ResetAccountAccessKey

class Account:
    def __init__(self, iam_client, cli_args):
        self.iam_client = iam_client
        self.cli_args = cli_args

    def create(self):
        if self.cli_args.name is None:
            print("Account name is required.")
            return

        if self.cli_args.email is None:
            print("Email Id of the user is required to create an Account")
            return

        body = urllib.parse.urlencode({'Action' : 'CreateAccount',
            'AccountName' : self.cli_args.name, 'Email' : self.cli_args.email})
        headers = {'content-type': 'application/x-www-form-urlencoded',
                'Accept': 'text/plain'}
        headers['Authorization'] = sign_request_v2('POST', '/', {}, headers)
        response = ConnMan.send_post_request(body, headers)
        if(response['status'] == 201):
            account = CreateAccountResponse(response)

            if account.is_valid_response():
                account.print_account_info()
            else:
                # unlikely message corruption case in network
                print("Account was created. For account details try account listing.")
                sys.exit(0)
        else:
            print("Account wasn't created.")
            error = ErrorResponse(response)
            error_message = error.get_error_message()
            print(error_message)

    # list all accounts
    def list(self):
        body = urllib.parse.urlencode({'Action' : 'ListAccounts'})
        headers = {'content-type': 'application/x-www-form-urlencoded',
                'Accept': 'text/plain'}
        headers['Authorization'] = sign_request_v2('POST', '/', {}, headers)
        response = ConnMan.send_post_request(body, headers)

        if response['status'] == 200:
            accounts = ListAccountResponse(response)

            if accounts.is_valid_response():
                accounts.print_account_listing()
            else:
                # unlikely message corruption case in network
                print("Failed to list accounts.")
                sys.exit(0)
        else:
            print("Failed to list accounts!")
            error = ErrorResponse(response)
            error_message = error.get_error_message()
            print(error_message)

    # delete account
    def delete(self):
        if self.cli_args.name is None:
            print('Account name is required.')
            return

        headers = {'content-type': 'application/x-www-form-urlencoded',
                'Accept': 'text/plain'}
        headers['Authorization'] = sign_request_v2('POST', '/', {}, headers)

        account_params = {'Action': 'DeleteAccount', 'AccountName': self.cli_args.name}
        if self.cli_args.force:
            account_params['force'] = True

        body = urllib.parse.urlencode(account_params)
        response = ConnMan.send_post_request(body, headers)

        if response['status'] == 200:
            print('Account deleted successfully.')
        else:
            print('Account cannot be deleted.')
            error = ErrorResponse(response)
            error_message = error.get_error_message()
            print(error_message)

    # Reset Account Access Key
    def reset_access_key(self):
        if self.cli_args.name is None:
            print("Account name is required.")
            return

        body = urllib.parse.urlencode({'Action' : 'ResetAccountAccessKey',
            'AccountName' : self.cli_args.name, 'Email' : self.cli_args.email})
        headers = {'content-type': 'application/x-www-form-urlencoded',
                'Accept': 'text/plain'}
        headers['Authorization'] = sign_request_v2('POST', '/', {}, headers)
        response = ConnMan.send_post_request(body, headers)
        if response['status'] == 201:
            account = ResetAccountAccessKey(response)

            if account.is_valid_response():
                account.print_account_info()
            else:
                # unlikely message corruption case in network
                print("Account access key was reset successfully.")
                sys.exit(0)
        else:
            print("Account access key wasn't reset.")
            error = ErrorResponse(response)
            error_message = error.get_error_message()
            print(error_message)
