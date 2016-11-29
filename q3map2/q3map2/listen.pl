#!/usr/bin/perl -w 

use IO::Socket;  
use Net::hostent; 

my $port = shift || 13131; 

my $server = IO::Socket::INET->new(
	Proto => 'tcp', 
	LocalPort => $port, 
	Listen => SOMAXCONN, 
	Reuse => 1 )
	|| die "can't setup server"; 
print "[Q3Map2 listener $0 is now active on port $port]\n"; 

while( $client = $server->accept() )
{ 

	$client->autoflush( 1 ); 
	
	$hostinfo = gethostbyaddr( $client->peeraddr );
	printf "[Connect from %s]\n\n", $hostinfo ? $hostinfo->name : $client->peerhost; 
	
	#ask the client for a command 
	print $client "[server]\$";
	my $text = "";
	while( <$client> )
	{
		$text .= $_;
		while( $text =~ s|<message[^>]*>([^<]+)</message>|| )
		{
			my $msg = $1;
			
			# fix xml ents
			$msg =~ s|&lt;|<|g;
			$msg =~ s|&gt;|>|g;
			$msg =~ s|&quot;|"|g;#"
			$msg =~ s|&apos;|'|g;#'
		
			print $msg;
		}
	}
	
	printf "\n[Disconnected: %s]\n\n", $hostinfo ? $hostinfo->name : $client->peerhost; 
	close $client;
} 
