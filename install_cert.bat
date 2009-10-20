:: this should add the certificate as a Trusted Root Certificate Authority and Trusted Publishers so that items signed by this particular certificate would be recognized
CertMgr /add testcert.cer /s /r localMachine root
CertMgr /add testcert.cer /s /r localMachine trustedpublisher
