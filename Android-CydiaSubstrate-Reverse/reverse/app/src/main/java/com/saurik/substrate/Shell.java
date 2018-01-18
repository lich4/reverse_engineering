package com.saurik.substrate;

import android.util.Log;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.Writer;

public class Shell {

    private static class Gobbler extends Thread {
        private BufferedReader in_;
        private boolean log_;
        private Writer out_;
        private String tag_;
        private boolean written_;

        public Gobbler(InputStream in, Writer out, String tag, boolean log) {
            this.in_ = new BufferedReader(new InputStreamReader(in));
            this.out_ = out;
            this.tag_ = tag;
            this.log_ = log;
        }

        public void run() {
            String line;
            try {
                while ((line = this.in_.readLine()) != null && line.length() != 0) {
                    try {
                        this.written_ = true;
                        if (this.log_)
                            android.util.Log.e("CydiaSubstrate", "su." + this.tag_ + ": " + line);
                        if (this.out_ != null)
                            this.out_.write(line);
                        this.out_.write('\n');
                    }
                    catch (IOException e) {
                        this.in_.close();
                        return;
                    }
                }
            }
            catch(IOException e2){
            }
            try {
                this.in_.close();
            }
            catch (IOException e) {
            }
        }

        public boolean wasWritten() {
            return this.written_;
        }
    }

    public static String quote(String value) {
        int start = 0;
        int end = value.length();
        StringBuilder builder = new StringBuilder((end + 2) + 3);
        builder.append('\'');
        while (start != end) {
            int quote = value.indexOf(39, start);
            if (quote == -1) {
                builder.append(value, start, end);
                break;
            }
            builder.append(value, start, quote);
            builder.append("'\\''");
            start = quote + 1;
        }
        builder.append('\'');
        return builder.toString();
    }

    private static void joinThread(Thread thread) {
        if (thread != null) {
            boolean interrupted = false;
            while (true) {
                try {
                    thread.join();
                    break;
                } catch (InterruptedException e) {
                    interrupted = true;
                }
            }
            if (interrupted) {
                Thread.currentThread().interrupt();
            }
        }
    }

    public static int exec(String shell, Writer stdout, Writer stderr, String... args) throws IOException {
        Process process = Runtime.getRuntime().exec(shell);
        Gobbler output = new Gobbler(process.getInputStream(), stdout, "stdout", false);
        output.start();
        Gobbler error;
        try {
            error = new Gobbler(process.getErrorStream(), stderr, "stderr", true);
            error.start();
            Writer out;
            try {
                int waitFor;
                out = new OutputStreamWriter(process.getOutputStream());
                boolean space = false;
                for (String arg : args) {
                    if (space) {
                        out.write(" ");
                    } else {
                        space = true;
                    }
                    out.write(quote(arg));
                }
                out.write("\n");
                out.flush();
                out.close();
                boolean interrupted = false;
                while (true) {
                    try {
                        waitFor = process.waitFor();
                        break;
                    } catch (InterruptedException e) {
                        Thread.interrupted();
                        interrupted = true;
                    }
                }
                if (waitFor != 0) {
                    Log.e("CydiaSubstrate", "su.status: " + waitFor);
                }
                if (interrupted) {
                    Thread.currentThread().interrupt();
                }
                if (waitFor == 0 && error.wasWritten()) {
                    waitFor = -1;
                    joinThread(output);
                    return waitFor;
                }
                joinThread(error);
                joinThread(output);
                return waitFor;
            } catch (Throwable th) {
                joinThread(error);
            }
        }
        finally {
            output = null;
        }
        return -1;
    }
}
