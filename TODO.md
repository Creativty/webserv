- [ ] CGI Path resolution
    - Assuming CGI route on /cgi
    - No other route even if it is CGI should be definable under /cgi/*
    - ```
      /cgi/my_script.sh/whatever/blabla
      <===============><==============>
         SCRIPT_NAME       PATH_INFO
    ```
- [ ] Client Timeouts
- [ ] Project directory de-cluttering
- [ ] Forbidden functions removal
- [ ] Pessimist code path testing
