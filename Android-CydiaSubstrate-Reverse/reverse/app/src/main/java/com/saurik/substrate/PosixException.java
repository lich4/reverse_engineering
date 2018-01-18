package com.saurik.substrate;

public class PosixException extends Exception {
    private int code_;
    private String file_;
    private int line_;

    public PosixException(int code, String file, int line) {
        this.code_ = code;
        this.file_ = file;
        this.line_ = line;
    }

    public int getCode() {
        return this.code_;
    }

    public String getFile() {
        return this.file_;
    }

    public int getLine() {
        return this.line_;
    }

    public String getMessage() {
        return Integer.toString(this.code_);
    }
}
